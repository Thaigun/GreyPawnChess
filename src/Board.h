#pragma once

#include <string>
#include <vector>

#include "GameState.h"
#include "Square.h"

class Board 
{
public:
    std::vector<std::string> findPossibleMoves();
    void applyMove(const std::string& move);
private:
    char getSquareIndex(const char* sqr);
    Square getSquare(const char* sqr);
    Square getSquare(char file, char rank);
    void setSquare(const char* sqr, Square data);
    void setSquare(char sqr, Square data);

    void findLegalMovesForSquare(char square, std::vector<std::string> &moveList);
    std::vector<std::string> findAllTargetsForSquare(char square);
    bool checkMoveLegality(const std::string& move);

    // Squares are in order from white's perspective left to right, bottom to top. 
    // a1, b1, c1 ... a1, b2, c2
    Square squares[64];
    Color playerInTurn = Color::WHITE;
    // These are saved in order to support Chess960 in the future.
    char kingRookFile = 0;
    char queenRookFile = 7;
    bool whiteCanCastleKing = true;
    bool whiteCanCastleQueen = true;
    bool blackCanCastleKing = true;
    bool blackCanCastleQueen = true;
    // Square which is available for an en passant take on this move.
    char enPassant = -1;
};