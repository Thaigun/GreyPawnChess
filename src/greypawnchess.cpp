#include "GreyPawnChess.h"
#include "GameState.h"

void GreyPawnChess::setup(char color, float timeSeconds, float incrementSeconds, char* variant) 
{

}

void GreyPawnChess::startGame()
{

}

void GreyPawnChess::updateGameState(GameState state)
{

}

void GreyPawnChess::setMoveCallback(std::function<void(std::string)> cb)
{
    moveCallback = cb;
}