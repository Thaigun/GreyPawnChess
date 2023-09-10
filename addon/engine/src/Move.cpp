#include "Move.h"
#include "BoardFuncs.h"

#include <iostream>

Move::Move()
{
    from[0] = -1; from[1] = -1;
    to[0] = -1; to[1] = -1;
}

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

Move::Move(const std::string& uciString)
{
    from[0] = BoardFuncs::getSquareIndex(uciString.substr(0, 2));
    to[0] = BoardFuncs::getSquareIndex(uciString.substr(2, 2));
    if (uciString.length() == 5)
    {
        if (uciString[4] == 'q')
            promotion = Piece::QUEEN;
        else if (uciString[4] == 'n')
            promotion = Piece::KNIGHT;
        else if (uciString[4] == 'b')
            promotion = Piece::BISHOP;
        else if (uciString[4] == 'r')
            promotion = Piece::ROOK;
    }
}

bool Move::isCastling() const
{
    return to[1] != -1;
}

bool Move::isPromotion() const
{
    return promotion != Piece::NONE;
}

bool Move::isValid() const
{
    if (from[0] == -1)
        return false;

    if (to[1] != -1 && from[1] == -1)
        return false;

    return true;
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