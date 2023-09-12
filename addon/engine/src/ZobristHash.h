#pragma once

#include "GameState.h"
#include "Piece.h"

class ZobristHash
{
public:
    void initHash(
        Piece* pieces, 
        Color playerInTurn, 
        bool whiteCanCastleKing, 
        bool whiteCanCastleQueen, 
        bool blackCanCastleKing, 
        bool blackCanCastleQueen, 
        int enPassant
    );
    unsigned int getHash() const;
    void toggleCastlingRights(Color player, char kingOrQueen);
    void toggleEnPassant(int enPassantFile);
    void togglePiece(char square, Piece piece);
    void togglePlayerInTurn();
    static unsigned int* getZobristHashTable();
    static int zobristPieceKey(Piece piece);

private:
    unsigned int hash = 0u;
};
