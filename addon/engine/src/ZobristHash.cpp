#include "ZobristHash.h"

#include <assert.h>
#include <iostream>
#include <climits>

#include "Random.h"

void ZobristHash::initHash(
    Piece* pieces, 
    Color playerInTurn, 
    bool whiteCanCastleKing, 
    bool whiteCanCastleQueen, 
    bool blackCanCastleKing, 
    bool blackCanCastleQueen, 
    int enPassant
)
{
    hash = 0u;
    for (int i = 0; i < 64; i++)
    {
        if (pieces[i] != Piece::NONE)
        {
            int pieceIdx = zobristPieceKey(pieces[i]);
            hash ^= getZobristHashTable()[i * 12 + pieceIdx];
        }
    }
    if (playerInTurn == Color::WHITE)
    {
        hash ^= getZobristHashTable()[64 * 12];
    }
    if (whiteCanCastleKing)
    {
        hash ^= getZobristHashTable()[64 * 12 + 1];
    }
    if (whiteCanCastleQueen)
    {
        hash ^= getZobristHashTable()[64 * 12 + 2];
    }
    if (blackCanCastleKing)
    {
        hash ^= getZobristHashTable()[64 * 12 + 3];
    }
    if (blackCanCastleQueen)
    {
        hash ^= getZobristHashTable()[64 * 12 + 4];
    }
    if (enPassant != -1)
    {
        int enPassantFile = enPassant % 8;
        hash ^= getZobristHashTable()[64 * 12 + 5 + enPassantFile];
    }
}

unsigned int ZobristHash::getHash() const
{
    return hash;
}

void ZobristHash::toggleCastlingRights(Color player, char kingOrQueen)
{
    assert(kingOrQueen == 'k' || kingOrQueen == 'q');
    if (player == Color::WHITE)
    {
        if (kingOrQueen == 'k')
        {
            hash ^= getZobristHashTable()[12 * 64 + 1];
        }
        else
        {
            hash ^= getZobristHashTable()[12 * 64 + 2];
        }
    }
    else
    {
        if (kingOrQueen == 'k')
        {
            hash ^= getZobristHashTable()[12 * 64 + 3];
        }
        else
        {
            hash ^= getZobristHashTable()[12 * 64 + 4];
        }
    }
}

void ZobristHash::toggleEnPassant(int enPassantFile)
{
    hash ^= getZobristHashTable()[12 * 64 + 1 + 4 + enPassantFile];
}

void ZobristHash::togglePiece(char square, Piece piece)
{
    hash ^= getZobristHashTable()[(int)square * 12 + zobristPieceKey(piece)];
}

void ZobristHash::togglePlayerInTurn()
{
    hash ^= getZobristHashTable()[12 * 64];
}

int ZobristHash::zobristPieceKey(Piece piece) 
{
    switch (uint16_t(piece))
    {
    case (uint16_t(Piece::PAWN | Piece::WHITE)):
        return 0;
        break;
    case (uint16_t(Piece::KNIGHT | Piece::WHITE)):
        return 1;
        break;
    case (uint16_t(Piece::BISHOP | Piece::WHITE)):
        return 2;
        break;
    case (uint16_t(Piece::ROOK | Piece::WHITE)):
        return 3;
        break;
    case (uint16_t(Piece::QUEEN | Piece::WHITE)):
        return 4;
        break;
    case (uint16_t(Piece::KING | Piece::WHITE)):
        return 5;
        break;
    case (uint16_t(Piece::PAWN | Piece::BLACK)):
        return 6;
        break;
    case (uint16_t(Piece::KNIGHT | Piece::BLACK)):
        return 7;
        break;
    case (uint16_t(Piece::BISHOP | Piece::BLACK)):
        return 8;
        break;
    case (uint16_t(Piece::ROOK | Piece::BLACK)):
        return 9;
        break;
    case (uint16_t(Piece::QUEEN | Piece::BLACK)):
        return 10;
        break;
    case (uint16_t(Piece::KING | Piece::BLACK)):
        return 11;
        break;

    default:
        return 0;
        break;
    }
}

unsigned int* ZobristHash::getZobristHashTable()
{
    // 64 squares, 12 different pieces, 1 for player in turn, 4 for castling rights, 8 for en passant file
    static unsigned int zobristHashTable[64 * 12 + 1 + 4 + 8];
    static bool initialized = false;
    if (!initialized)
    {
        // If the hash is not yet calculated, calculate it.
        constexpr int nNumbers = 64 * 12 + 1 + 4 + 8;
        for (int i = 0; i < nNumbers; i++)
        {
            zobristHashTable[i] = Random::Range(0u, UINT_MAX);
        }
        initialized = true;
    }
    return zobristHashTable;
}