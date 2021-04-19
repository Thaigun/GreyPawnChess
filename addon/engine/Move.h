#pragma once

#include "Piece.h"

#include <string>

enum class MoveDirection : char
{
    N = 8, E = 1, W = -1, S = -8,
    NE = N+E, NW = N+W, SE=S+E, SW=S+W
};

struct Move
{
    Move(char from, char to);
    Move(char from1, char to1, char from2, char to2);
    Move(char from, char to, Piece prom);

    bool isCastling() const;
    std::string asUCIstr();
    
    char from[2];
    char to[2];
    Piece promotion = Piece::NONE;
};