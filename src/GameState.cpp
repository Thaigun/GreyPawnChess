#include "GameState.h"

GameState::GameState(int timeMs, int incrementMs)
    : wTime(timeMs)
    , bTime(timeMs)
    , wIncrement(incrementMs)
    , bIncrement(incrementMs)
    , status(GameStatus::CREATED)
{}

bool GameState::finishedStatus() 
{
    return static_cast<int>(status) >= 25;
}