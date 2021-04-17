#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "GameState.h"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;
typedef std::chrono::duration<float> Duration;

class GreyPawnChess 
{
public:
    // Setup the game
    void setup(char color, int timeMs, int incrementMs, const std::string& variant);
    // Starts calculating the best moves, i.e. playing the match
    void startGame();
    // Indicates that the game should stop and waits for the worker thread to stop.
    void stopGame();
    // Updates the current state of the game. Includes moves as well as game state (ended, resigned etc.)
    void updateGameState(GameState newState);
    // The engine is free to decide when it should make a move. 
    // Give it a callback function it should call when it wants to make the move.
    void setMoveCallback(std::function<void(const std::string&)> cb);

private:
    // Private methods
    Duration timeSinceStateSet();
    Color playerInTurn();

    // Members used to manage the worker thread and by the worker thread.
    std::function<void(std::string)> moveCallback;
    std::thread workThread;
    std::mutex mtx;
    std::atomic<bool> running = false;

    // Members related to the setup of the game.
    Color myColor;
    std::string variant;

    // Members to store the known state of the game.
    GameState gameState;
    TimePoint stateSetTime;

    bool debugBool = false;
};
