#pragma once

#include <string>
#include <vector>

// Reference of these status codes as of 15.4.2021: 
// https://github.com/ornicar/scalachess/blob/0a7d6f2c63b1ca06cd3c958ed3264e738af5c5f6/src/main/scala/Status.scala#L16-L28
enum class GameStatus 
{
    CREATED = 10,
    STARTED = 20,
    // From this point the game is finished
    ABORTED = 25,
    MATE    = 30,
    RESIGN  = 31,
    STALEMATE = 32,
    TIMEOUT = 33,
    DRAW    = 34,
    OUTOFTIME = 35,
    CHEAT   = 36,
    NOSTART = 37,
    UNKNOWN_FINISH = 38,
    VARIANT_END = 60
};

enum Color 
{
    WHITE, BLACK, NONE
};

struct GameState 
{
    GameState() = default;
    GameState(int timeMs, int incrementMs);
    GameState(int whiteTime, int blackTime, int whiteIncrement, int blackIncrement, 
        GameStatus gameStatus, Color winnerCol, std::string& movesUpdate);
    
    bool finishedStatus();

    // milliseconds left for black and white.
    int wTime;
    int bTime;
    int wIncrement;
    int bIncrement;
    GameStatus status;
    Color winner = Color::NONE;
    std::vector<std::string> moves;
};