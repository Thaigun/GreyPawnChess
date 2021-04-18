#include "Board.h"

#include <assert.h>
#include <utility>

bool Board::checkMoveLegality(const std::string& move)
{
    // TODO: Implement
    return true;
}

std::vector<std::string> Board::findAllTargetsForSquare(char square)
{
    std::vector<std::string> targets;
    Piece data = pieces[square];
    char rank = square / 8;
    char file = square % 8;
    // TODO: Implement possible piece movements.
    if (!!(data & Piece::PAWN))
    {
        char nextRank = !!(data & Piece::WHITE) ? rank + 1 : rank - 1;
        if (nextRank > 7 || nextRank < 0)
        {
            return targets;
        }
    }
    else if (!!(data & Piece::KNIGHT))
    {

    }
    else if (!!(data & Piece::BISHOP))
    {

    }
    else if (!!(data & Piece::ROOK))
    {
        
    }
    else if (!!(data & Piece::QUEEN))
    {
        
    }
    else if (!!(data & Piece::KING))
    {
        
    }
    return targets;
}

void Board::findLegalMovesForSquare(char square, std::vector<std::string> &moveList)
{
    std::vector<std::string> unfilteredMoves = findAllTargetsForSquare(square);
    for (const std::string& move : unfilteredMoves)
    {
        if (checkMoveLegality(move))
        {
            moveList.push_back(move);
        }
    }
}

std::vector<std::string> Board::findPossibleMoves() 
{
    std::vector<std::string> foundMoves;
    for (char sqr = 0; sqr < 63; sqr++)
    {
        findLegalMovesForSquare(sqr, foundMoves);
    }
}
    
void Board::applyMove(const std::string& move) 
{
    assert(move.size() >= 4);
    const char* moveStr = move.c_str();

    const char firstSquareIdx = getSquareIndex(moveStr);
    const char secondSquareIdx = getSquareIndex(&moveStr[2]);
    const Piece firstSquareData = getSquare(moveStr);
    const Piece secondSquareData = getSquare(&moveStr[2]);
    Piece newSquareData = firstSquareData;

    // Handle potential promotion
    if (!!(firstSquareData & Piece::PAWN))
    {
        const char promotionRank = !!(firstSquareData & Piece::WHITE) ? 0 : 7;
        const char pawnRank = secondSquareIdx / 8;
        if (promotionRank == pawnRank) 
        {
            assert(move.size() >= 5);
            newSquareData &= ~Piece::PAWN;
            switch (move[4]) 
            {
                case 'q':
                    newSquareData |= Piece::QUEEN;
                    break;
                case 'r':
                    newSquareData |= Piece::ROOK;
                    break;
                case 'n':
                    newSquareData |= Piece::KNIGHT;
                    break;
                case 'b':
                    newSquareData |= Piece::BISHOP;
                    break;
                default:
                    newSquareData |= Piece::QUEEN;
                    break;
            }
        }
    }

    // Handle castling
    bool castles = false;
    if (!!(firstSquareData & Piece::KING)) 
    {
        // Normally, king moving from the original position to the castling square means castling. 
        if (!!(firstSquareData & Piece::WHITE))
        {
            if (firstSquareIdx == getSquareIndex("e1")) 
            {
                if (secondSquareIdx == getSquareIndex("g1"))
                {
                    // White castles king side
                    castles = true;
                    Piece rookSquare = getSquare("h1");
                    setSquare("h1", Piece::NONE);
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("f1", rookSquare);
                }
                else if (secondSquareIdx == getSquareIndex("c1"))
                {
                    // White castles queen side
                    castles = true;
                    Piece rookSquare = getSquare("a1");
                    setSquare("a1", Piece::NONE);
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("d1", rookSquare);
                }
            }
        }
        if (!!(firstSquareData & Piece::BLACK))
        {
            if (firstSquareIdx == getSquareIndex("e8")) 
            {
                if (secondSquareIdx == getSquareIndex("g8"))
                {
                    // Black castles king side
                    castles = true;
                    Piece rookSquare = getSquare("h8");
                    setSquare("h8", Piece::NONE);
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("f8", rookSquare);
                }
                else if (secondSquareIdx == getSquareIndex("c8"))
                {
                    // Black castles queen side
                    castles = true;
                    Piece rookSquare = getSquare("a8");
                    setSquare("a8", Piece::NONE);
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("d8", rookSquare);
                }
            }
        }

        // In Fischer960 variation castling is denoted by a king taking his own rook.
        if (!!(secondSquareData & Piece::ROOK))
        {
            if (!!(firstSquareData & Piece::WHITE) && !!(secondSquareData & Piece::WHITE))
            {
                if (secondSquareIdx % 8 > firstSquareIdx % 8)
                {
                    // White castles king side
                    castles = true;
                    Piece rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, Piece::NONE);
                    setSquare("g1", newSquareData);
                    setSquare("f1", rookSquare);
                }
                else if (secondSquareIdx % 8 < firstSquareIdx % 8)
                {
                    // White castles queen side
                    castles = true;
                    Piece rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, Piece::NONE);
                    setSquare("c1", newSquareData);
                    setSquare("d1", rookSquare);
                }
            }
            if (!!(firstSquareData & Piece::BLACK) && !!(secondSquareData & Piece::BLACK))
            {
                if (secondSquareIdx % 8 > firstSquareIdx % 8)
                {
                    // Black castles king side
                    castles = true;
                    Piece rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, Piece::NONE);
                    setSquare("g8", newSquareData);
                    setSquare("f8", rookSquare);
                }
                else if (secondSquareIdx % 8 < firstSquareIdx % 8)
                {
                    // Black castles queen side
                    castles = true;
                    Piece rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Piece::NONE);
                    setSquare(secondSquareIdx, Piece::NONE);
                    setSquare("c8", newSquareData);
                    setSquare("d8", rookSquare);
                }
            }
        }
    }

    if (!castles)
    {
        setSquare(secondSquareIdx, newSquareData);
        setSquare(firstSquareIdx, Piece::NONE);
    }
    
    // Mark changes in ability to castle.
    if (!!(firstSquareData & Piece::KING))
    {
        if (!!(firstSquareData & Piece::WHITE))
        {
            whiteCanCastleKing = false;
            whiteCanCastleQueen = false;
        }
        if (!!(firstSquareData & Piece::BLACK)) 
        {
            blackCanCastleKing = false;
            blackCanCastleQueen = false;
        }
    }

    if (!!(firstSquareData & Piece::ROOK))
    {
        // If the rook was on its starting square and has now moved, cannot castle that side anymore
        if (!!(firstSquareData & Piece::WHITE))
        {
            char whiteQRookStartIdx = queenRookFile;
            char whiteKRookStartIdx = kingRookFile;
            if (firstSquareIdx == whiteQRookStartIdx)
                whiteCanCastleQueen = false;
            else if (firstSquareIdx == whiteKRookStartIdx)
                whiteCanCastleKing = false;
        }
        else if (!!(firstSquareData & Piece::BLACK))
        {
            char blackQRookStartIdx = 8 * 7 + queenRookFile;
            char blackKRookStartIdx = 8 * 7 + kingRookFile;
            if (firstSquareIdx == blackQRookStartIdx)
                blackCanCastleQueen = false;
            else if (firstSquareIdx == blackKRookStartIdx)
                blackCanCastleKing = false;
        }
    }
    if (!!(secondSquareData & Piece::ROOK))
    {
        // If a rook that was on its starting square is taken, we cannot castle that side anymore
        if (!!(secondSquareData & Piece::WHITE))
        {
            char whiteQRookStartIdx = queenRookFile;
            char whiteKRookStartIdx = kingRookFile;
            if (secondSquareIdx == whiteQRookStartIdx)
                whiteCanCastleQueen = false;
            else if (secondSquareIdx == whiteKRookStartIdx)
                whiteCanCastleKing = false;
        }
        else if (!!(secondSquareData & Piece::BLACK))
        {
            char blackQRookStartIdx = 8 * 7 + queenRookFile;
            char blackKRookStartIdx = 8 * 7 + kingRookFile;
            if (secondSquareIdx == blackQRookStartIdx)
                blackCanCastleQueen = false;
            else if (secondSquareIdx == blackKRookStartIdx)
                blackCanCastleKing = false;
        }
    }

    // Update en passant possiblity
    enPassant = -1; // Disable by default, enable if needed.
    if (!!(firstSquareData & Piece::PAWN))
    {
        // For white pieces, the square in front is 8 indices ahead, for black it's -8.
        char rankForwardOffset = !!(firstSquareData & Piece::WHITE) ? 8 : -8;
        if (secondSquareIdx == firstSquareIdx + 2 * rankForwardOffset) 
        {
            enPassant = firstSquareIdx + rankForwardOffset;
        }
    }

    // Update whose turn it is
    playerInTurn = playerInTurn == Color::BLACK ? Color::WHITE : Color::BLACK;
}

char Board::getSquareIndex(const char* sqr)
{
    // char 'a' equals to 97, followed by 'b'=97, 'c'=99...
    char file = sqr[0] - (char)97;
    // char '0' equals to 48, followed by '1'=49, '2'=50...
    char rank = sqr[1] - (char)49;
    return 8 * rank + file;
}

void Board::setSquare(const char* sqr, Piece data)
{
    pieces[getSquareIndex(sqr)] = data;
}

void Board::setSquare(char sqr, Piece data)
{
    pieces[sqr] = data;
}

Piece Board::getSquare(const char* sqr)
{
    return pieces[getSquareIndex(sqr)];
}

Piece Board::getSquare(char file, char rank)
{
    return pieces[8 * rank + file];
}