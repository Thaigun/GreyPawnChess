#include "Move.h"

Move::Move(char from1, char to1) 
{
    from[0] = from1; from[1] = -1;
    to[0] = from1; to[1] = -1;
}

Move::Move(char from1, char to1, char from2, char to2) 
{
    from[0] = from1; from[1] = from2;
    to[0] = from1; to[1] = to2;
}

Move::Move(char from1, char to1, Piece prom)
{
    from[0] = from1; from[1] = -1;
    to[0] = from1; to[1] = -1;
    promotion = prom;
}

bool Move::isCastling() const
{
    return from[1] != -1;
}