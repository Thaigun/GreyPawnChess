#include "GameState.h"

#include <sstream>
#include <algorithm>
#include <iterator>

GameState::GameState(int timeMs, int incrementMs)
    : wTime(timeMs)
    , bTime(timeMs)
    , wIncrement(incrementMs)
    , bIncrement(incrementMs)
    , status(GameStatus::CREATED)
{}

GameState::GameState(int whiteTime, int blackTime, int whiteIncrement, int blackIncrement, 
    GameStatus gameStatus, Color winnerCol, std::string& movesUpdate)
    : wTime(whiteTime)
    , bTime(blackTime)
    , wIncrement(whiteIncrement)
    , bIncrement(blackIncrement)
    , status(gameStatus)
    , winner(winnerCol)
{
    std::istringstream iss(movesUpdate);
    std::copy(
        std::istream_iterator<std::string>(iss), 
        std::istream_iterator<std::string>(), 
        std::back_inserter(moves)
    );
}

bool GameState::finishedStatus() 
{
    return static_cast<int>(status) >= 25;
}