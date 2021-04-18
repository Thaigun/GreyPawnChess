#pragma once

#include <string>
#include <vector>

#include "GameState.h"
#include "Piece.h"

struct Move;

class Board 
{
public:
    std::vector<Move> findPossibleMoves();
    Move constructMove(const std::string& moveUCI);
    void applyMove(const Move& move);
private:
    Piece getSquare(const char* sqr);
    Piece getSquare(char file, char rank);
    void setSquare(const char* sqr, Piece data);
    void setSquare(char sqr, Piece data);
    char stepSquareInDirection(char square, MoveDirection direction);

    void updateCastlingRights(Color color);
    void findLegalMovesForSquare(char square, std::vector<Move> &moveList);
    std::vector<Move> findPseudoLegalMoves(char square, Color forPlayer, bool pawnOnlyTakes = false);
    bool checkMoveLegality(const Move& move);
    bool isThreatened(char square, Color byPlayer);
    bool areSameColor(Piece p1, Piece p2);

    Move constructPromotionMove(const std::string& moveUCI);
    Move constructCastlingMove(char firstSquare, char secondSquare);

    std::vector<Move> findPseudoPawnMoves(char square, Color player, bool onlyTakes = false);
    std::vector<Move> findPseudoRookMoves(char square, Color player);
    std::vector<Move> findPseudoQueenMoves(char square, Color player);
    std::vector<Move> findPseudoBishopMoves(char square, Color player);
    std::vector<Move> findPseudoKingMoves(char square, Color player);
    std::vector<Move> findPseudoKnightMoves(char square, Color player);
    std::vector<Move> findDirectionalPseudoMoves(char square, const std::vector<MoveDirection>& directions, int maxSteps = 1000000);

    // Squares are in order from white's perspective left to right, bottom to top. 
    // a1, b1, c1 ... a1, b2, c2
    Piece pieces[64];
    Color playerInTurn = Color::WHITE;
    // These are saved in order to support Chess960 in the future.
    char kingRookFile = 0;
    char queenRookFile = 7;
    char kingStartFile = 4;
    bool whiteCanCastleKing = true;
    bool whiteCanCastleQueen = true;
    bool blackCanCastleKing = true;
    bool blackCanCastleQueen = true;
    // Square which is available for an en passant take on this move.
    char enPassant = -1;
};