#include "Move.h"
#include "BoardFuncs.h"

#include <iostream>

Move::Move(char from1, char to1) 
{
    from[0] = from1; from[1] = -1;
    to[0] = to1; to[1] = -1;
}

Move::Move(char from1, char to1, char from2, char to2) 
{
    from[0] = from1; from[1] = from2;
    to[0] = to1; to[1] = to2;
}

Move::Move(char from1, char to1, Piece prom)
{
    from[0] = from1; from[1] = -1;
    to[0] = to1; to[1] = -1;
    promotion = prom;
}

bool Move::isCastling() const
{
    return to[1] != -1;
}

bool Move::isPromotion() const
{
    return promotion != Piece::NONE;
}

std::string Move::asUCIstr() const
{
    std::string moveString = BoardFuncs::squareToString(from[0]) + BoardFuncs::squareToString(to[0]);
    if (promotion != Piece::NONE)
    {
        if (!!(promotion & Piece::QUEEN))
            moveString += "q";
        else if (!!(promotion & Piece::KNIGHT))
            moveString += "n";
        else if (!!(promotion & Piece::BISHOP))
            moveString += "b";
        else if (!!(promotion & Piece::ROOK))
            moveString += "r";
    }
    return moveString;
}