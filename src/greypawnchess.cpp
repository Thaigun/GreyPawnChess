#include "GreyPawnChess.h"

#include <iostream>
#include "GameState.h"

void GreyPawnChess::setup(char color, float timeSeconds, float incrementSeconds, char* variant) 
{

}

void GreyPawnChess::startGame()
{
    int v = 0;
    for (int i = 0; i < 100000000; i++) {
        if (i % 2) {
            v += 1;
        }
        if (i % 3 == 0) {
            v /= 2;
        }
    }
    moveCallback(std::string("e2e4"));
    std::cout << v << std::endl;
}

void GreyPawnChess::updateGameState(GameState state)
{

}

void GreyPawnChess::setMoveCallback(std::function<void(std::string)> cb)
{
    moveCallback = cb;
}