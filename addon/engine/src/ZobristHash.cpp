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
    std::cout << "toggleCastlingRights ";
    assert(kingOrQueen == 'k' || kingOrQueen == 'q');
    if (player == Color::WHITE)
    {
        if (kingOrQueen == 'k')
        {
            std::cout << "white king";
            hash ^= getZobristHashTable()[12 * 64 + 1];
        }
        else
        {
            std::cout << "white queen";
            hash ^= getZobristHashTable()[12 * 64 + 2];
        }
    }
    else
    {
        if (kingOrQueen == 'k')
        {
            std::cout << "black king";
            hash ^= getZobristHashTable()[12 * 64 + 3];
        }
        else
        {
            std::cout << "black queen";
            hash ^= getZobristHashTable()[12 * 64 + 4];
        }
    }
    std::cout << std::endl;
}

void ZobristHash::toggleEnPassant(int enPassantFile)
{
    std::cout << "toggleEnPassant " << enPassantFile << std::endl;
    hash ^= getZobristHashTable()[12 * 64 + 1 + 4 + enPassantFile];
}

void ZobristHash::togglePiece(char square, Piece piece)
{
    std::cout << "togglePiece " << (int)square;
    hash ^= getZobristHashTable()[(int)square * 12 + zobristPieceKey(piece)];
    std::cout << std::endl;
}

void ZobristHash::togglePlayerInTurn()
{
    std::cout << "togglePlayerInTurn" << std::endl;
    hash ^= getZobristHashTable()[12 * 64];
}

int ZobristHash::zobristPieceKey(Piece piece) 
{
    switch (uint16_t(piece))
    {
    case (uint16_t(Piece::PAWN | Piece::WHITE)):
        std::cout << "white pawn";
        return 0;
        break;
    case (uint16_t(Piece::KNIGHT | Piece::WHITE)):
        std::cout << "white knight";
        return 1;
        break;
    case (uint16_t(Piece::BISHOP | Piece::WHITE)):
        std::cout << "white bishop";
        return 2;
        break;
    case (uint16_t(Piece::ROOK | Piece::WHITE)):
        std::cout << "white rook";
        return 3;
        break;
    case (uint16_t(Piece::QUEEN | Piece::WHITE)):
        std::cout << "white queen";
        return 4;
        break;
    case (uint16_t(Piece::KING | Piece::WHITE)):
        std::cout << "white king";
        return 5;
        break;
    case (uint16_t(Piece::PAWN | Piece::BLACK)):
        std::cout << "black pawn";
        return 6;
        break;
    case (uint16_t(Piece::KNIGHT | Piece::BLACK)):
        std::cout << "black knight";
        return 7;
        break;
    case (uint16_t(Piece::BISHOP | Piece::BLACK)):
        std::cout << "black bishop";
        return 8;
        break;
    case (uint16_t(Piece::ROOK | Piece::BLACK)):
        std::cout << "black rook";
        return 9;
        break;
    case (uint16_t(Piece::QUEEN | Piece::BLACK)):
        std::cout << "black queen";
        return 10;
        break;
    case (uint16_t(Piece::KING | Piece::BLACK)):
        std::cout << "black king";
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