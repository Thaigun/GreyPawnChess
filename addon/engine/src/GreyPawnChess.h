#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Board.h"
#include "GameState.h"
#include "Move.h"
#include "TimeManagement.h"

class GreyPawnChess 
{
public:
    // Setup the game
    void setup(char color, int timeMs, int incrementMs, const std::string& setupVariant);
    // Starts calculating the best moves, i.e. playing the match
    void startGame();
    // Indicates that the game should stop and waits for the worker thread to stop.
    void stopGame();
    // Updates the current state of the game. Includes moves as well as game state (ended, resigned etc.)
    void updateGameState(GameState newState);
    // The engine is free to decide when it should make a move. 
    // Give it a callback function it should call when it wants to make the move.
    void setMoveCallback(std::function<void(const std::string&)> cb);
    // A callback the engine calls to indicate the game is ended.
    void setGameEndReasonCallback(std::function<void(const std::string&)> cb);

protected:
    // To be oberriden by specific strategy implementations
    // Called repeatedly while the game is running. Each tick should take max 200ms.
    virtual void tickComputation() = 0;
    virtual void applyMoveToStrategy(const Move& move) = 0;
    virtual Move getBestMove() = 0;

    Duration timeSinceStateSet();
    Color playerInTurn();
    void applyMove(const Move& move);
    void makeComputerMove(const Move& move);

    Board board;
    std::vector<Move> moves;

    // Members used to manage the worker thread and by the worker thread.
    std::function<void(std::string)> moveCallback;
    bool gameEndCallBackSet = false;
    std::function<void(std::string)> gameEndReasonCallback;
    std::thread workThread;
    std::mutex mtx;
    std::atomic<bool> running = false;

    // How sure are we this is the best move, [0.0, 1.0].
    float confidence = 0.0f;

    // Members related to the setup of the game.
    Color myColor;
    std::string variant;

    // Members to store the known state of the game.
    GameState gameState;
    TimePoint stateSetTime;
};
