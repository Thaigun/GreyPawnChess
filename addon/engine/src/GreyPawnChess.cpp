#include "GreyPawnChess.h"

#include <assert.h>
#include <atomic>
#include <iostream>
#include <math.h>

#include "GameState.h"
#include "Random.h"

#define MTX_LOCK std::unique_lock<std::mutex> lock(mtx);

void GreyPawnChess::setup(char color, int timeMs, int incrementMs, const std::string& variant) 
{
    MTX_LOCK
    myColor = color == 'w' ? Color::WHITE : Color::BLACK;
    gameState = GameState(timeMs, incrementMs);
    this->variant = variant;
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
                    const std::string newMove = gameState.moves[moves.size()];
                    moves.push_back(board.constructMove(newMove));
                }
            }

            // Apply new moves on the board.
            while (movesApplied < moves.size())
            {
                board.applyMove(moves[moves.size() - 1]);
                movesApplied++;
                std::vector<Move> possibleMoves = board.findPossibleMoves();
                if (possibleMoves.size() == 0)
                    return;
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
            
            if (TimeManagement::timeToMove(timeLeftMs, increment, movesApplied, timeSpent, confidence))
            {
                makeComputerMove(monteCarloTree.getBestOption());
            }
        }
    });
}

void GreyPawnChess::tickComputation()
{
    // IMPLEMENT THIS TO MAKE DA STRONK ENGINE

    // Run a few iterations of the Monte Carlo search.
    for (int i = 0; i < 100; i++)
    {
        monteCarloTree.runIteration();
    }
    
    // IMPLEMENTATION ENDS HERE

    // This MUST be set in this method if it's our turn.
    confidence = 1.0f;
}

void GreyPawnChess::makeComputerMove(const Move& move)
{
    board.applyMove(move);
    moves.push_back(move);
    movesApplied++;
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