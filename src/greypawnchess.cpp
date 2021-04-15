#include "GreyPawnChess.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <math.h>

#include "GameState.h"

#define MTX_LOCK std::unique_lock<std::mutex> lock(mtx);

void GreyPawnChess::setup(char color, float timeSeconds, float incrementSeconds, const std::string& variant) 
{

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
            float v = 0.0f;
            // Calculate the best move based on current game state.
            for (int i = 0; i < 1000000000; i++) {
                if (i % 2) {
                    v += 1;
                }
                if (i % 3 == 0) {
                    v /= 2;
                }
            }
            // Post the best move to the callback.
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

void GreyPawnChess::updateGameState(GameState state)
{

}

void GreyPawnChess::setMoveCallback(std::function<void(const std::string&)> cb)
{
    moveCallback = cb;
}