#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "GameState.h"
#include "Move.h"
#include "Piece.h"
#include "ZobristHash.h"

class Board 
{
public:
    Board();
    static Board buildFromFEN(const std::string& fenString);

    std::vector<Move> findPossibleMoves() const;
    void findPinnedPieceMoves(char pinnedPieceSquare, MoveDirection pinDirection, std::vector<Move> &moves) const;
    Move constructMove(const std::string &moveUCI) const;
    void applyMove(const Move& move);
    void applyMove(const std::string& moveUCI);
    Piece getSquare(char square) const;
    Piece getSquare(const char* sqr) const;
    Piece getSquare(char file, char rank) const;
    Color getCurrentPlayer() const;
    bool isCheck() const;
    bool isMate() const;
    bool insufficientMaterial() const;
    bool noProgress() const;
    bool threefoldRepetition() const;
    std::string getFEN() const;
    unsigned int getHash();

private:
    void setSquare(const char* sqr, Piece data);
    void setSquare(char sqr, Piece data);
    static char stepSquareInDirection(char square, MoveDirection direction);

    void updateCastlingRights();
    void findLegalMovesForSquare(char square, std::vector<Move>& moveList) const;
    bool checkKingMoveLegality(const Move& move) const;
    Piece findPieceInDirection(char square, MoveDirection direction, char *pieceSquare) const;
    std::vector<char> findKnightThreats(char square, Piece byColor) const;
    bool posesXrayThreat(Piece piece, MoveDirection direction, int distance) const;
    bool checkMoveLegality(const Move &move) const;
    char findSquareWithPiece(Piece piece) const;
    bool isThreatened(char square, Color byPlayer) const;
    bool hasPawnThreat(char square, Color byPlayer) const;
    unsigned char turnsSincePawnMoveOrCapture() const;
    static bool areSameColor(Piece p1, Piece p2);

    Move constructPromotionMove(const std::string& moveUCI) const;
    Move constructCastlingMove(char firstSquare, char secondSquare) const;
    Move constructEnPassantMove(char firstSSquare, char secondSquare) const;

    void findPseudoLegalMoves(char square, Color forPlayer, std::vector<Move>& pseudoMoves, bool pawnOnlyTakes = false, bool forceIncludePawnTakes = false) const;
    void findPseudoPawnMoves(char square, Color player, std::vector<Move>& moves, bool onlyTakes = false, bool forceIncludeTakes = false) const;
    void findPseudoRookMoves(char square, std::vector<Move>& moves) const;
    void findPseudoQueenMoves(char square, std::vector<Move>& moves) const;
    void findPseudoBishopMoves(char square, std::vector<Move>& moves) const;
    void findPseudoCastlingMoves(char square, Color player, std::vector<Move>& moves) const;
    void findPseudoKingMoves(char square, Color player, std::vector<Move>& moves, bool includeCastling = true) const;
    void findPseudoKnightMoves(char square, std::vector<Move>& moves) const;
    void findDirectionalPseudoMoves(char square, const std::vector<MoveDirection>& directions, std::vector<Move>& moves, int maxSteps = 1000000) const;

    void updateRepetitionHistory();
    void resetRepetitionHistory();

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

    ZobristHash hash;
    
    unsigned int repeatablePositionsWhite[50];
    unsigned char whitePositionsSize = 0u;

    unsigned int repeatablePositionsBlack[50];
    unsigned char blackPositionsSize = 0u;
    
    unsigned char highestRepetitionCount = 0u;
};