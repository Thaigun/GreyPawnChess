#pragma once

#include "Piece.h"

#include <string>

enum class MoveDirection : char
{
    N = 8, E = 1, W = -1, S = -8,
    NE = N+E, NW = N+W, SE=S+E, SW=S+W
};

MoveDirection operator-(MoveDirection direction)
{
    return static_cast<MoveDirection>(-static_cast<char>(direction));
}

struct Move
{
    Move();
    Move(char from, char to);
    Move(char from1, char to1, char from2, char to2);
    Move(char from, char to, Piece prom);

    bool operator==(const Move& other)
    {
        return 
            from[0] == other.from[0] &&
            from[1] == other.from[1] &&
            to[0] == other.to[0] && 
            to[1] == other.to[1] &&
            promotion == other.promotion;
    } 

    bool isCastling() const;
    bool isPromotion() const;
    std::string asUCIstr() const;
    // Doesn't check if the move can happen in a game, only checks that it makes sense representation-wise.
    bool isValid() const;
    
    char from[2];
    char to[2];
    Piece promotion = Piece::NONE;
};