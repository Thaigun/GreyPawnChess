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
        // Check if the match is still running.
        while (running) 
        {
            Color currentPlayer;
            {
                MTX_LOCK;
                currentPlayer = playerInTurn();
            }
            if (currentPlayer != myColor)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            moveCallback(std::string("e2e4"));
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