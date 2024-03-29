#include "GreyPawnChess.h"

#include <assert.h>
#include <atomic>
#include <iostream>
#include <math.h>

#include "GameState.h"
#include "Random.h"
#include "ScopedProfiler.h"

#define MTX_LOCK std::unique_lock<std::mutex> lock(mtx);

void GreyPawnChess::setup(char color, int timeMs, int incrementMs, const std::string& setupVariant) 
{
    MTX_LOCK
    myColor = color == 'w' ? Color::WHITE : Color::BLACK;
    gameState = GameState(timeMs, incrementMs);
    variant = setupVariant;
}

void GreyPawnChess::startGame()
{
    {
        MTX_LOCK
        running = true;
    }
    workThread = std::thread([this]() {
        // Check if the match is still running.
        while (running) 
        {
            // Construct moves that have come since the last update.
            {
                MTX_LOCK
                while (moves.size() < gameState.moves.size())
                {
                    PROFILER_RESET();
                    const std::string newMove = gameState.moves[moves.size()];
                    Move move = board.constructMove(newMove);
                    applyMove(move);
                    std::vector<Move> possibleMoves = board.findPossibleMoves();
                    if (possibleMoves.size() == 0)
                    {
                        if (gameEndCallbackSet)
                            board.isCheck() ? gameEndReasonCallback("lose") : gameEndReasonCallback("draw");
                        return;
                    }
                    if (board.threefoldRepetition() || board.insufficientMaterial() || board.noProgress())
                    {
                        if (gameEndCallbackSet)
                            gameEndReasonCallback("draw");
                        return;
                    }
                }
            }

            // Tick the computation.
            TimePoint tickStartTime = std::chrono::system_clock::now();
            this->tickComputation();
            Duration tickDuration = std::chrono::system_clock::now() - tickStartTime;
            if (tickDuration.count() > 0.2f)
                std::cout << "Computation tick took more than 200 milliseconds: " << tickDuration.count() * 1000 << std::endl;

            // If waiting for the other player, we can continue computating.
            if (playerInTurn() != myColor)
            {
                continue;
            }

            // Tick _must_ update the best option if it's our turn.
            float lastTimeLeft = float(myColor == Color::WHITE ? gameState.wTime : gameState.bTime);
            float increment = float(myColor == Color::WHITE ? gameState.wIncrement : gameState.bIncrement);
            Duration timeSpent = timeSinceStateSet();
            float timeLeftMs = float(lastTimeLeft) - timeSpent.count() * 1000;
            
            if (TimeManagement::timeToMove(timeLeftMs, increment, (int)moves.size(), timeSpent, confidence))
            {
                PROFILER_PRINT();
                PROFILER_RESET();

                makeComputerMove(getBestMove());
                std::vector<Move> possibleMoves = board.findPossibleMoves();
                if (possibleMoves.size() == 0)
                {
                    MTX_LOCK
                    if (gameEndCallbackSet)
                        board.isCheck() ? gameEndReasonCallback("checkmate") : gameEndReasonCallback("draw");
                    running = false;
                }
                if (board.threefoldRepetition() || board.insufficientMaterial() || board.noProgress())
                {
                    MTX_LOCK
                    if (gameEndCallbackSet)
                        gameEndReasonCallback("draw");
                    running = false;
                }
            }
        }
    });
}

void GreyPawnChess::applyMove(const Move& move)
{
    moves.push_back(move);
    board.applyMove(move);
    applyMoveToStrategy(move);
}

void GreyPawnChess::makeComputerMove(const Move& move)
{
    applyMove(move);
    {
        MTX_LOCK
        moveCallback(move.asUCIstr());
    }
}

void GreyPawnChess::stopGame()
{
    if (!running)
        return;

    {
        MTX_LOCK
        running = false;
    }
    // Wait for the worker thread to quit. It should happen when it detects that running flag is false.
    workThread.join();
}

void GreyPawnChess::updateGameState(GameState newState)
{
    MTX_LOCK
    gameState = newState;
    stateSetTime = std::chrono::system_clock::now();
}

void GreyPawnChess::setMoveCallback(std::function<void(const std::string&)> cb)
{
    MTX_LOCK
    moveCallback = cb;
}

void GreyPawnChess::setGameEndReasonCallback(std::function<void(const std::string&)> cb)
{
    MTX_LOCK
    gameEndCallbackSet = true;
    gameEndReasonCallback = cb;
}

Duration GreyPawnChess::timeSinceStateSet() 
{
    return std::chrono::system_clock::now() - stateSetTime;
}

Color GreyPawnChess::playerInTurn() 
{
    if (gameState.finishedStatus())
        return Color::NONE;

    return moves.size() % 2 == 0 ? Color::WHITE : Color::BLACK;
}