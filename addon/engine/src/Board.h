#pragma once

#include <string>
#include <vector>

#include "GameState.h"
#include "Move.h"
#include "Piece.h"

class Board 
{
public:
    Board();
    static Board buildFromFEN(const std::string& fenString);

    std::vector<Move> findPossibleMoves() const;
    Move constructMove(const std::string& moveUCI);
    void applyMove(const Move& move);
    Piece getSquare(const char* sqr) const;
    Piece getSquare(char file, char rank) const;
    Color getCurrentPlayer() const;
    bool isCheck() const;
    bool isMate() const;
    bool insufficientMaterial() const;
    std::string getFEN() const;
    unsigned int getHash();

private:
    void setSquare(const char* sqr, Piece data);
    void setSquare(char sqr, Piece data);
    static char stepSquareInDirection(char square, MoveDirection direction);

    void updateCastlingRights();
    void findLegalMovesForSquare(char square, std::vector<Move> &moveList) const;
    bool checkMoveLegality(const Move& move) const;
    char findSquareWithPiece(Piece piece) const;
    bool isThreatened(char square, Color byPlayer) const;
    static bool areSameColor(Piece p1, Piece p2);

    Move constructPromotionMove(const std::string& moveUCI);
    Move constructCastlingMove(char firstSquare, char secondSquare);

    void findPseudoLegalMoves(char square, Color forPlayer, std::vector<Move>& pseudoMoves, bool pawnOnlyTakes = false, bool forceIncludePawnTakes = false) const;
    void findPseudoPawnMoves(char square, Color player, std::vector<Move>& moves, bool onlyTakes = false, bool forceIncludeTakes = false) const;
    void findPseudoRookMoves(char square, std::vector<Move>& moves) const;
    void findPseudoQueenMoves(char square, std::vector<Move>& moves) const;
    void findPseudoBishopMoves(char square, std::vector<Move>& moves) const;
    void findPseudoCastlingMoves(char square, Color player, std::vector<Move>& moves) const;
    void findPseudoKingMoves(char square, Color player, std::vector<Move>& moves) const;
    void findPseudoKnightMoves(char square, std::vector<Move>& moves) const;
    void findDirectionalPseudoMoves(char square, const std::vector<MoveDirection>& directions, std::vector<Move>& moves, int maxSteps = 1000000) const;

    void initHash();
    static unsigned int* getZobristHashTable();
    static int zobristPieceKey(Piece piece);

    // Squares are in order from white's perspective left to right, bottom to top. 
    // a1, b1, c1 ... a2, b2, c2
    Piece pieces[64];
    Color playerInTurn = Color::WHITE;
    // These are saved in order to support Chess960 in the future.
    char kingRookFile = 7;
    char queenRookFile = 0;
    char kingStartFile = 4;
    bool whiteCanCastleKing = true;
    bool whiteCanCastleQueen = true;
    bool blackCanCastleKing = true;
    bool blackCanCastleQueen = true;
    // Square which is available for an en passant take on this move.
    char enPassant = -1;

    unsigned int hash = 0;
};