#include "GreyPawnChess.h"

#include <atomic>
#include <iostream>
#include <math.h>

#include "GameState.h"

void GreyPawnChess::setup(char color, float timeSeconds, float incrementSeconds, char* variant) 
{

}

void GreyPawnChess::startGame()
{
    running = true;
    workThread = std::thread([this]() {
        float v = 0.0f;
        while (running) 
        {
            for (int i = 0; i < 1000000000; i++) {
                if (i % 2) {
                    v += 1;
                }
                if (i % 3 == 0) {
                    v /= 2;
                }
            }
            moveCallback(std::string("e2e4"));
        }
    });
}

void GreyPawnChess::stopGame()
{
    if (!running)
        return;

    running = false;
    workThread.join();
}

void GreyPawnChess::updateGameState(GameState state)
{

}

void GreyPawnChess::setMoveCallback(std::function<void(const std::string&)> cb)
{
    moveCallback = cb;
}