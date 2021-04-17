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
    Square data = squares[square];
    char rank = square / 8;
    char file = square % 8;
    // TODO: Implement possible piece movements.
    if (!!(data & Square::PAWN))
    {
        char nextRank = !!(data & Square::WHITE) ? rank + 1 : rank - 1;
        if (nextRank > 7 || nextRank < 0)
        {
            return targets;
        }
    }
    else if (!!(data & Square::KNIGHT))
    {

    }
    else if (!!(data & Square::BISHOP))
    {

    }
    else if (!!(data & Square::ROOK))
    {
        
    }
    else if (!!(data & Square::QUEEN))
    {
        
    }
    else if (!!(data & Square::KING))
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
    const Square firstSquareData = getSquare(moveStr);
    const Square secondSquareData = getSquare(&moveStr[2]);
    Square newSquareData = firstSquareData;

    // Handle potential promotion
    if (!!(firstSquareData & Square::PAWN))
    {
        const char promotionRank = !!(firstSquareData & Square::WHITE) ? 0 : 7;
        const char pawnRank = secondSquareIdx / 8;
        if (promotionRank == pawnRank) 
        {
            assert(move.size() >= 5);
            newSquareData &= ~Square::PAWN;
            switch (move[4]) 
            {
                case 'q':
                    newSquareData |= Square::QUEEN;
                    break;
                case 'r':
                    newSquareData |= Square::ROOK;
                    break;
                case 'n':
                    newSquareData |= Square::KNIGHT;
                    break;
                case 'b':
                    newSquareData |= Square::BISHOP;
                    break;
                default:
                    newSquareData |= Square::QUEEN;
                    break;
            }
        }
    }

    // Handle castling
    bool castles = false;
    if (!!(firstSquareData & Square::KING)) 
    {
        // Normally, king moving from the original position to the castling square means castling. 
        if (!!(firstSquareData & Square::WHITE))
        {
            if (firstSquareIdx == getSquareIndex("e1")) 
            {
                if (secondSquareIdx == getSquareIndex("g1"))
                {
                    // White castles king side
                    castles = true;
                    Square rookSquare = getSquare("h1");
                    setSquare("h1", Square::NONE);
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("f1", rookSquare);
                }
                else if (secondSquareIdx == getSquareIndex("c1"))
                {
                    // White castles queen side
                    castles = true;
                    Square rookSquare = getSquare("a1");
                    setSquare("a1", Square::NONE);
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("d1", rookSquare);
                }
            }
        }
        if (!!(firstSquareData & Square::BLACK))
        {
            if (firstSquareIdx == getSquareIndex("e8")) 
            {
                if (secondSquareIdx == getSquareIndex("g8"))
                {
                    // Black castles king side
                    castles = true;
                    Square rookSquare = getSquare("h8");
                    setSquare("h8", Square::NONE);
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("f8", rookSquare);
                }
                else if (secondSquareIdx == getSquareIndex("c8"))
                {
                    // Black castles queen side
                    castles = true;
                    Square rookSquare = getSquare("a8");
                    setSquare("a8", Square::NONE);
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, newSquareData);
                    setSquare("d8", rookSquare);
                }
            }
        }

        // In Fischer960 variation castling is denoted by a king taking his own rook.
        if (!!(secondSquareData & Square::ROOK))
        {
            if (!!(firstSquareData & Square::WHITE) && !!(secondSquareData & Square::WHITE))
            {
                if (secondSquareIdx % 8 > firstSquareIdx % 8)
                {
                    // White castles king side
                    castles = true;
                    Square rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, Square::NONE);
                    setSquare("g1", newSquareData);
                    setSquare("f1", rookSquare);
                }
                else if (secondSquareIdx % 8 < firstSquareIdx % 8)
                {
                    // White castles queen side
                    castles = true;
                    Square rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, Square::NONE);
                    setSquare("c1", newSquareData);
                    setSquare("d1", rookSquare);
                }
            }
            if (!!(firstSquareData & Square::BLACK) && !!(secondSquareData & Square::BLACK))
            {
                if (secondSquareIdx % 8 > firstSquareIdx % 8)
                {
                    // Black castles king side
                    castles = true;
                    Square rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, Square::NONE);
                    setSquare("g8", newSquareData);
                    setSquare("f8", rookSquare);
                }
                else if (secondSquareIdx % 8 < firstSquareIdx % 8)
                {
                    // Black castles queen side
                    castles = true;
                    Square rookSquare = secondSquareData;
                    setSquare(firstSquareIdx, Square::NONE);
                    setSquare(secondSquareIdx, Square::NONE);
                    setSquare("c8", newSquareData);
                    setSquare("d8", rookSquare);
                }
            }
        }
    }

    if (!castles)
    {
        setSquare(secondSquareIdx, newSquareData);
        setSquare(firstSquareIdx, Square::NONE);
    }
    
    // Mark changes in ability to castle.
    if (!!(firstSquareData & Square::KING))
    {
        if (!!(firstSquareData & Square::WHITE))
        {
            whiteCanCastleKing = false;
            whiteCanCastleQueen = false;
        }
        if (!!(firstSquareData & Square::BLACK)) 
        {
            blackCanCastleKing = false;
            blackCanCastleQueen = false;
        }
    }

    if (!!(firstSquareData & Square::ROOK))
    {
        // If the rook was on its starting square and has now moved, cannot castle that side anymore
        if (!!(firstSquareData & Square::WHITE))
        {
            char whiteQRookStartIdx = queenRookFile;
            char whiteKRookStartIdx = kingRookFile;
            if (firstSquareIdx == whiteQRookStartIdx)
                whiteCanCastleQueen = false;
            else if (firstSquareIdx == whiteKRookStartIdx)
                whiteCanCastleKing = false;
        }
        else if (!!(firstSquareData & Square::BLACK))
        {
            char blackQRookStartIdx = 8 * 7 + queenRookFile;
            char blackKRookStartIdx = 8 * 7 + kingRookFile;
            if (firstSquareIdx == blackQRookStartIdx)
                blackCanCastleQueen = false;
            else if (firstSquareIdx == blackKRookStartIdx)
                blackCanCastleKing = false;
        }
    }
    if (!!(secondSquareData & Square::ROOK))
    {
        // If a rook that was on its starting square is taken, we cannot castle that side anymore
        if (!!(secondSquareData & Square::WHITE))
        {
            char whiteQRookStartIdx = queenRookFile;
            char whiteKRookStartIdx = kingRookFile;
            if (secondSquareIdx == whiteQRookStartIdx)
                whiteCanCastleQueen = false;
            else if (secondSquareIdx == whiteKRookStartIdx)
                whiteCanCastleKing = false;
        }
        else if (!!(secondSquareData & Square::BLACK))
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
    if (!!(firstSquareData & Square::PAWN))
    {
        // For white pieces, the square in front is 8 indices ahead, for black it's -8.
        char rankForwardOffset = !!(firstSquareData & Square::WHITE) ? 8 : -8;
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

void Board::setSquare(const char* sqr, Square data)
{
    squares[getSquareIndex(sqr)] = data;
}

void Board::setSquare(char sqr, Square data)
{
    squares[sqr] = data;
}

Square Board::getSquare(const char* sqr)
{
    return squares[getSquareIndex(sqr)];
}

Square Board::getSquare(char file, char rank)
{
    return squares[8 * rank + file];
}