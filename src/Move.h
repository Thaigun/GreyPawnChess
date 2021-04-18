#include "Piece.h"

enum class MoveDirection : char
{
    N = 8, E = 1, W = -1, S = -8,
    NE = N+E, NW = N+W, SE=S+E, SW=S+W
};

struct Move
{
    char from[2];
    char to[2];
    Piece promotion = Piece::NONE;
};