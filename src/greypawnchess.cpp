#include "GreyPawnChess.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <math.h>

#include "GameState.h"

#define MTX_LOCK std::unique_lock<std::mutex> lock(mtx);

void GreyPawnChess::setup(char color, int timeMs, int incrementMs, const std::string& variant) 
{
    MTX_LOCK
    myColor = color == 'w' ? WHITE : BLACK;
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
        // TODO: This whole function will change, this is the placeholder for picking random moves
        // Check if the match is still running.
        while (running) 
        {
            Color currentPlayer;
            // Lock while reading the state for current player.
            {
                MTX_LOCK;
                currentPlayer = playerInTurn();
            }
            // If waiting for the other player, just idle.
            if (currentPlayer != myColor)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            // Lock while reading the game state again.
            {
                MTX_LOCK
                if (moves.size() < gameState.moves.size())
                {
                    const std::string newMove = gameState.moves[gameState.moves.size() - 1];
                    moves.push_back(board.constructMove(newMove));
                }
            }
            if (movesApplied < moves.size())
            {
                board.applyMove(moves[moves.size() - 1]);
                movesApplied++;
            }
            
            std::vector<Move> possibleMoves = board.findPossibleMoves();

            if (possibleMoves.size() == 0)
                return;

            std::uniform_int_distribution<int> distribution(0, possibleMoves.size() - 1);
            int randomIdx = distribution(rng);
            moveCallback(possibleMoves[randomIdx].asUCIstr());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
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
        return NONE;

    return gameState.moves.size() % 2 == 0 ? WHITE : BLACK;
}