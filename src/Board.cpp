#include "Board.h"

#include <algorithm>
#include <assert.h>
#include <utility>

#include "BoardFuncs.h"
#include "Move.h"

Move Board::constructPromotionMove(const std::string& moveUCI)
{
    const char* moveStr = moveUCI.c_str();
    const char firstSquareIdx = BoardFuncs::getSquareIndex(moveStr);
    const char secondSquareIdx = BoardFuncs::getSquareIndex(&moveStr[2]);
    Move move = {
        { firstSquareIdx, -1 },
        { secondSquareIdx, -1 }
    };
    switch (moveStr[4])
    {
        case 'q':
            move.promotion = Piece::QUEEN;
            break;
        case 'b':
            move.promotion = Piece::BISHOP;
            break;
        case 'n':
            move.promotion = Piece::KNIGHT;
            break;
        case 'r':
            move.promotion = Piece::ROOK;
            break;
        default:
            move.promotion = Piece::QUEEN;
            break;    
    }
    return move;
}

Move Board::constructCastlingMove(char firstSquare, char secondSquare)
{
    char rank = playerInTurn == Color::WHITE ? 0 : 7;
    char rookFile = firstSquare > secondSquare ? queenRookFile : kingRookFile;
    char rookSquare = 8 * rank + rookFile;
    char kingTargetFile = firstSquare < secondSquare ? 6 : 2;
    char rookTargetFile = firstSquare < secondSquare ? 5 : 3;
    Move move = {
        {firstSquare, rookSquare},
        {8 * rank + kingTargetFile, 8 * rank + rookTargetFile}
    };
    return move;
}

Move Board::constructMove(const std::string& moveUCI)
{
    assert(moveUCI.size() >= 4);

    if (moveUCI.size() >= 5)
    {
        return constructPromotionMove(moveUCI);
    }

    const char* moveStr = moveUCI.c_str();
    const char firstSquareIdx = BoardFuncs::getSquareIndex(moveStr);
    const char secondSquareIdx = BoardFuncs::getSquareIndex(&moveStr[2]);
    const Piece firstSquareData = pieces[firstSquareIdx];
    const Piece secondSquareData = pieces[secondSquareIdx];
    bool sameColor = !!(firstSquareData & secondSquareData & Piece::COLOR_MASK);
    bool kingMove = !!(firstSquareData & Piece::KING);
    bool normalCastling = kingMove && std::abs(firstSquareIdx - secondSquareIdx) == 2;
    if (normalCastling || (sameColor && kingMove))
    {
        return constructCastlingMove(firstSquareIdx, secondSquareIdx);
    }
    Move move = {
        {firstSquareIdx, -1},
        {secondSquareIdx, -1}
    };
    return move;
}

void Board::applyMove(const Move& move) 
{
    // Reset, it will be set again if needed
    enPassant = -1;

    // In 960, the king might be moving where the rook is at the moment. For this reason,
    // we store both pieces before moving anything, to not lose the piece data.
    Piece movePieces[2];
    for (int i = 0; i < 2; i++)
    {
        char from = move.from[i];
        char to = move.to[i];
        if (from < 0 || to < 0)
            continue;

        movePieces[i] = pieces[from];
    }

    for (int i = 0; i < 2; i++)
    {
        char from = move.from[i];
        char to = move.to[i];
        if (from < 0 || to < 0)
            continue;

        Piece movePiece = movePieces[i];
        // Prepare potential promotion
        if (move.promotion != Piece::NONE)
        {
            // Keep the color but change the piece type.
            movePiece = (Piece::COLOR_MASK & movePiece) | move.promotion;
        }

        // Update en passant square
        if (!!(movePiece & Piece::PAWN))
        {
            // Moved to ranks to either direction?
            if (std::abs(from - to) == (char)16)
            {
                // En passant square is between from and to.
                enPassant = (from + to) / 2;
            }
        }
        pieces[from] = Piece::NONE;
        pieces[to] = movePiece;
    }

    updateCastlingRights(playerInTurn);

    // Update whose turn it is
    playerInTurn = playerInTurn == Color::BLACK ? Color::WHITE : Color::BLACK;
}

void Board::updateCastlingRights(Color color)
{
    // Update for white
    if (color == Color::WHITE)
    {
        if (whiteCanCastleQueen)
        {
            char qRookOrigSquare = queenRookFile;
            if (pieces[qRookOrigSquare] != (Piece::WHITE | Piece::ROOK))
                whiteCanCastleQueen = false;
        }
        if (whiteCanCastleKing)
        {
            char kRookOrigSquare = kingRookFile;
            if (pieces[kRookOrigSquare] != (Piece::WHITE | Piece::ROOK))
                whiteCanCastleKing = false;
        }
        if (whiteCanCastleKing || whiteCanCastleQueen)
        {
            char origKingSquare = kingStartFile;
            if (pieces[origKingSquare] != (Piece::WHITE | Piece::KING))
            {
                whiteCanCastleKing = false;
                whiteCanCastleQueen = false;
            }
        }
    }
    // Update for black
    if (color == Color::BLACK)
    {
        if (blackCanCastleQueen)
        {
            char qRookOrigSquare = queenRookFile;
            if (pieces[qRookOrigSquare] != (Piece::BLACK | Piece::ROOK))
                blackCanCastleQueen = false;
        }
        if (blackCanCastleKing)
        {
            char kRookOrigSquare = kingRookFile;
            if (pieces[kRookOrigSquare] != (Piece::BLACK | Piece::ROOK))
                blackCanCastleKing = false;
        }
        if (blackCanCastleKing || blackCanCastleQueen)
        {
            char origKingSquare = kingStartFile;
            if (pieces[origKingSquare] != (Piece::BLACK | Piece::KING))
            {
                blackCanCastleKing = false;
                blackCanCastleQueen = false;
            }
        }
    }
}

void Board::setSquare(const char* sqr, Piece data)
{
    pieces[BoardFuncs::getSquareIndex(sqr)] = data;
}

void Board::setSquare(char sqr, Piece data)
{
    pieces[sqr] = data;
}

Piece Board::getSquare(const char* sqr)
{
    return pieces[BoardFuncs::getSquareIndex(sqr)];
}

Piece Board::getSquare(char file, char rank)
{
    return pieces[8 * rank + file];
}