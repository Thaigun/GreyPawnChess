#pragma once

#include <functional>
#include <string>

struct GameState;

class GreyPawnChess 
{
public:
    // Setup the game
    void setup(char color, float timeSeconds, float incrementSeconds, char* variant);
    // Starts calculating the best moves, i.e. playing the match
    void startGame();
    // Updates the current state of the game. Includes moves as well as game state (ended, resigned etc.)
    void updateGameState(GameState state);
    // The engine is free to decide when it should make a move. 
    // Give it a callback function it should call when it wants to make the move.
    void setMoveCallback(std::function<void(std::string)> cb);
private:
    std::function<void(std::string)> moveCallback;
};
