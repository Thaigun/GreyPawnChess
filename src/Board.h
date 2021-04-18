#pragma once

#include <string>
#include <vector>

#include "GameState.h"
#include "Piece.h"

struct Move;

class Board 
{
public:
    std::vector<std::string> findPossibleMoves();
    Move constructMove(const std::string& moveUCI);
    void applyMove(const Move& move);
private:
    Piece getSquare(const char* sqr);
    Piece getSquare(char file, char rank);
    void setSquare(const char* sqr, Piece data);
    void setSquare(char sqr, Piece data);

    void updateCastlingRights(Color color);
    void findLegalMovesForSquare(char square, std::vector<std::string> &moveList);
    std::vector<std::string> findPseudoLegalMoves(char square);
    bool checkMoveLegality(const std::string& move);

    Move constructPromotionMove(const std::string& moveUCI);
    Move constructCastlingMove(char firstSquare, char secondSquare);

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