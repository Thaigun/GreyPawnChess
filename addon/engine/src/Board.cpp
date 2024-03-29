#include "Board.h"

#include <algorithm>
#include <assert.h>
#include <climits>
#include <iterator>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "BoardFuncs.h"
#include "Random.h"
#include "ScopedProfiler.h"
#include "StringUtil.h"

Board::Board()
{
    // Set up the standard variation board.
    Piece whitePawn = Piece::PAWN | Piece::WHITE;
    for (char square = 8; square < 16; square++)
    {
        pieces[square] = whitePawn;
    }
    Piece blackPawn = Piece::PAWN | Piece::BLACK;
    for (char square = 6 * 8; square < 7 * 8; square++)
    {
        pieces[square] = blackPawn;
    }

    char ranks[2] = { 0, 7 };
    Piece colors[2] = { Piece::WHITE, Piece::BLACK };
    Piece majorPieces[8] = {
        Piece::ROOK, Piece::KNIGHT, Piece::BISHOP, Piece::QUEEN,
        Piece::KING, Piece::BISHOP, Piece::KNIGHT, Piece::ROOK
    };
    for (char side = 0; side < 2; side++)
    {
        for (char file = 0; file < 8; file++)
        {
            pieces[ranks[side] * 8 + file] = colors[side] | majorPieces[file];
        }
    }

    for (char square = 16; square < 6 * 8; square++)
    {
        pieces[square] = Piece::NONE;
    }

    hash.initHash(pieces, playerInTurn, whiteCanCastleKing, whiteCanCastleQueen, blackCanCastleKing, blackCanCastleQueen, enPassant);

    updateRepetitionHistory();
}

Board Board::buildFromFEN(const std::string& fenString)
{
    Board newBoard;
    for (int i = 0; i < 64; i++)
        newBoard.pieces[i] = Piece::NONE;

    // Split the string by spaces.
    std::vector<std::string> fenParts = StringUtil::split(fenString);
    std::vector<std::string> boardPieces = StringUtil::split(fenParts[0], '/');
    
    // FEN Notation starts from a8. Put all pieces on the board.
    char currentSquare = 7 * 8;
    for (const std::string& rank : boardPieces)
    {
        for (const char& piece : rank)
        {
            // char 56 = '8'
            if (piece <= 56)
            {
                currentSquare += piece - 48;
                continue;
            }
            if (piece == 'p')
            {
                newBoard.pieces[currentSquare++] = Piece::BLACK | Piece::PAWN;
            }
            else if (piece == 'P')
            {
                newBoard.pieces[currentSquare++] = Piece::WHITE | Piece::PAWN;
            }
            else if (piece == 'n')
            {
                newBoard.pieces[currentSquare++] = Piece::BLACK | Piece::KNIGHT;
            }
            else if (piece == 'N')
            {
                newBoard.pieces[currentSquare++] = Piece::WHITE | Piece::KNIGHT;
            }
            else if (piece == 'b')
            {
                newBoard.pieces[currentSquare++] = Piece::BLACK | Piece::BISHOP;
            }
            else if (piece == 'B')
            {
                newBoard.pieces[currentSquare++] = Piece::WHITE | Piece::BISHOP;
            }
            else if (piece == 'r')
            {
                newBoard.pieces[currentSquare++] = Piece::BLACK | Piece::ROOK;
            }
            else if (piece == 'R')
            {
                newBoard.pieces[currentSquare++] = Piece::WHITE | Piece::ROOK;
            }
            else if (piece == 'q')
            {
                newBoard.pieces[currentSquare++] = Piece::BLACK | Piece::QUEEN;
            }
            else if (piece == 'Q')
            {
                newBoard.pieces[currentSquare++] = Piece::WHITE | Piece::QUEEN;
            }
            else if (piece == 'k')
            {
                newBoard.pieces[currentSquare++] = Piece::BLACK | Piece::KING;
            }
            else if (piece == 'K')
            {
                newBoard.pieces[currentSquare++] = Piece::WHITE | Piece::KING;
            }
        }
        // Step to the beginning of the previous rank.
        currentSquare -= 16;
    }
    newBoard.playerInTurn = fenParts[1].front() == 'w' ? Color::WHITE : Color::BLACK;
    
    // In the notation, castling rights are disabled unless specificly enabled.
    newBoard.whiteCanCastleKing = false;
    newBoard.whiteCanCastleQueen = false;
    newBoard.blackCanCastleKing = false;
    newBoard.blackCanCastleQueen = false;
    for (const char& castlingRight : fenParts[2])
    {
        if (castlingRight == 'k')
            newBoard.blackCanCastleKing = true;
        else if (castlingRight == 'K')
            newBoard.whiteCanCastleKing = true;
        else if (castlingRight == 'q')
            newBoard.blackCanCastleQueen = true;
        else if (castlingRight == 'Q')
            newBoard.whiteCanCastleQueen = true;
    }

    // Detect en passant availability
    if (!(fenParts[3].front() == '-'))
    {
        newBoard.enPassant = BoardFuncs::getSquareIndex(fenParts[3]);
    }
    newBoard.hash.initHash(
        newBoard.pieces, 
        newBoard.playerInTurn, 
        newBoard.whiteCanCastleKing, 
        newBoard.whiteCanCastleQueen, 
        newBoard.blackCanCastleKing, 
        newBoard.blackCanCastleQueen, 
        newBoard.enPassant
    );

    newBoard.resetRepetitionHistory();
    newBoard.updateRepetitionHistory();
    
    return newBoard;
}

std::vector<Move> Board::findPossibleMoves() const
{
    //PROFILE("Board::findPossibleMoves");

    std::vector<Move> moves;
    moves.reserve(64);
    Piece currentPlayerColor = playerInTurn == Color::WHITE ? Piece::WHITE : Piece::BLACK;
    Piece opponentPieceColor = ~currentPlayerColor & Piece::COLOR_MASK;
    char kingSquare = findSquareWithPiece(currentPlayerColor | Piece::KING);
    assert(kingSquare >= 0 && kingSquare < 64 && "King must be on the board.");

    bool specialTreatmentSquares[64];
    for (int i = 0; i < 63; i++)
        specialTreatmentSquares[i] = false;

    specialTreatmentSquares[kingSquare] = true;
    char pinnedPieces[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    MoveDirection directions[8] = {
        MoveDirection::N,
        MoveDirection::S,
        MoveDirection::E,
        MoveDirection::W,
        MoveDirection::NE,
        MoveDirection::SE,
        MoveDirection::SW,
        MoveDirection::NW
    };
    std::vector<char> checkingPieces = findKnightThreats(kingSquare, opponentPieceColor);
    for (int i = 0; i < 8; i++)
    {
        char ownPieceSquare = -1;
        char nextSquare = stepSquareInDirection(kingSquare, directions[i]);
        int stepCount = 1;
        while (nextSquare != -1)
        {
            Piece nextSquarePiece = pieces[nextSquare];
            if (areSameColor(currentPlayerColor, nextSquarePiece))
            {
                if (ownPieceSquare != -1)
                    break;
                ownPieceSquare = nextSquare;
            }
            else if (nextSquarePiece != Piece::NONE)
            {
                // Check if the opponent piece in the square can move to the king's square
                // disregarding other pieces blocking.
                MoveDirection oppositeDir = static_cast<MoveDirection>(-char(directions[i]));
                if (posesXrayThreat(nextSquarePiece, oppositeDir, stepCount))
                {
                    if (ownPieceSquare != -1)
                    {
                        pinnedPieces[i] = ownPieceSquare;
                        specialTreatmentSquares[ownPieceSquare] = true;
                    }
                    else 
                    {
                        checkingPieces.push_back(nextSquare);
                    }
                }
                // In case of an en passantable pawn a threat may come through two pieces horizontally, then don't break.
                if (
                    (nextSquarePiece & ~Piece::COLOR_MASK) != Piece::PAWN || 
                    enPassant % 8 != nextSquare % 8
                )
                {
                    break;
                }
            }
            nextSquare = stepSquareInDirection(nextSquare, directions[i]);
            stepCount++;
        }
    }

    if (checkingPieces.size() > 1)
    {
        // King is in double check, only king moves are legal.
        std::vector<Move> candidateKingMoves;
        candidateKingMoves.reserve(8);
        findPseudoKingMoves(kingSquare, playerInTurn, candidateKingMoves, false);
        Color opponentColor = playerInTurn == Color::WHITE ? Color::BLACK : Color::WHITE; 
        for (const Move& move : candidateKingMoves)
        {
            if (!isThreatened(move.to[0], opponentColor))
            {
                moves.push_back(move);
            }
        }
        return moves;
    }
    
    std::vector<Move> candidateMoves;
    const bool isCheck = checkingPieces.size() > 0;
    for (char square = 0; square < 64; square++)
    {
        const bool isSpecialSquare = specialTreatmentSquares[square];
        findPseudoLegalMoves(square, playerInTurn, (isSpecialSquare || isCheck) ? candidateMoves : moves, false, false);
    }

    for (const Move& move : candidateMoves)
    {
        if (checkMoveLegality(move))
        {
            moves.push_back(move);
        }
    }

    return moves;
}

void Board::findPinnedPieceMoves(char pinnedPieceSquare, MoveDirection pinDirection, std::vector<Move>& moves) const
{
    // Pinned piece can only move in the pin direction.
    Piece pinnedPiece = pieces[pinnedPieceSquare];
    Piece pinnedPieceType = pinnedPiece & ~Piece::COLOR_MASK;
    MoveDirection oppositeDir = static_cast<MoveDirection>(-char(pinDirection));
    const bool isDiagonal = pinDirection == MoveDirection::NE || pinDirection == MoveDirection::SE || pinDirection == MoveDirection::SW || pinDirection == MoveDirection::NW;
    switch (pinnedPieceType)
    {
        case Piece::PAWN:
            if (isDiagonal)
            {
                // Pinned pawn can only move diagonally to take the piece pinning it.
                char takePieceSquare = stepSquareInDirection(pinnedPieceSquare, pinDirection);
                if (pieces[takePieceSquare] != Piece::NONE)
                {
                    moves.push_back(Move(pinnedPieceSquare, takePieceSquare));
                }
            }
            else if (pinDirection == MoveDirection::N || pinDirection == MoveDirection::S)
            {
                // Pinned pawn can only move forward one step.
                MoveDirection forward = playerInTurn == Color::WHITE ? MoveDirection::N : MoveDirection::S;
                char nextSquare = stepSquareInDirection(pinnedPieceSquare, forward);
                if (pieces[nextSquare] == Piece::NONE)
                {
                    moves.push_back(Move(pinnedPieceSquare, nextSquare));
                    if (pinnedPieceSquare % 8 == 1 || pinnedPieceSquare % 8 == 6)
                    {
                        // Double step for a pawn?
                        char nextnextSquare = stepSquareInDirection(nextSquare, forward);
                        if (pieces[nextnextSquare] == Piece::NONE)
                            moves.push_back(Move(pinnedPieceSquare, nextnextSquare));
                    }
                }
            }
            return;
        case Piece::ROOK:
            if (!isDiagonal)
                findDirectionalPseudoMoves(pinnedPieceSquare, { pinDirection, oppositeDir }, moves);
            return;
        case Piece::QUEEN:
            findDirectionalPseudoMoves(pinnedPieceSquare, { pinDirection, oppositeDir }, moves);
            return;
        case Piece::BISHOP:
            if (isDiagonal)
                findDirectionalPseudoMoves(pinnedPieceSquare, { pinDirection, oppositeDir }, moves);
            return;
        default:
            return;
    }
}

Piece Board::findPieceInDirection(char square, MoveDirection direction, char* pieceSquare) const
{
    char nextSquare = stepSquareInDirection(square, direction);
    while (nextSquare != -1)
    {
        Piece nextSquarePiece = pieces[nextSquare];
        if (nextSquarePiece != Piece::NONE)
        {
            *pieceSquare = nextSquare;
            return nextSquarePiece;
        }
        nextSquare = stepSquareInDirection(nextSquare, direction);
    }
    *pieceSquare = -1;
    return Piece::NONE;
} 

std::vector<char> Board::findKnightThreats(char square, Piece byColor) const
{
    char file = square % 8;
    char rank = square / 8;
    char rankOffsets[8] = {  1, 2,2,1,-1,-2,-2,-1 };
    char fileOffsets[8] = { -2,-1,1,2, 2, 1,-1,-2 };
    std::vector<char> threats;
    for (int i = 0; i < 8; i++)
    {
        char moveRank = rank + rankOffsets[i];
        char moveFile = file + fileOffsets[i];
        if (moveRank > 7 || moveRank < 0 || moveFile < 0 || moveFile > 7)
            continue;
        
        char moveSquare = 8 * moveRank + moveFile;
        Piece targetSquarePiece = pieces[moveSquare];
        if (targetSquarePiece == (Piece::KNIGHT | byColor))
            threats.push_back(moveSquare);
    }
    return threats;
}

bool Board::posesXrayThreat(Piece piece, MoveDirection direction, int distance) const
{
    Piece pieceType = piece & ~Piece::COLOR_MASK;
    Piece pieceColor = piece & Piece::COLOR_MASK;
    switch (pieceType)
    {
    case Piece::PAWN:
        if (distance > 1)
            return false;
        if (pieceColor == Piece::WHITE)
            return direction == MoveDirection::NW || direction == MoveDirection::NE;
        return direction == MoveDirection::SW || direction == MoveDirection::SE;
    case Piece::ROOK:
        return direction == MoveDirection::N || direction == MoveDirection::S || direction == MoveDirection::E || direction == MoveDirection::W;
    case Piece::QUEEN:
        return true;
    case Piece::BISHOP:
        return direction == MoveDirection::NE || direction == MoveDirection::SE || direction == MoveDirection::SW || direction == MoveDirection::NW;
    case Piece::KING:
        return distance == 1;
    default:
        return false;
    }
}

char Board::findSquareWithPiece(Piece piece) const
{
    for (char i = 0; i < 64; i++)
    {
        if (pieces[i] == piece)
            return i;
    }
    return -1;
}

void Board::findLegalMovesForSquare(char square, std::vector<Move> &moveList) const 
{
    std::vector<Move> pseudoMoves;
    findPseudoLegalMoves(square, playerInTurn, pseudoMoves);
    for (const Move& move : pseudoMoves)
    {
        if (checkMoveLegality(move))
        {
            moveList.push_back(move);
        }
    }
}

bool Board::checkKingMoveLegality(const Move& move) const
{
    return checkMoveLegality(move);
}

bool Board::checkMoveLegality(const Move& move) const
{
    PROFILE("Board::checkMoveLegality");
    if (move.isCastling()) 
    {
        char startSqr = std::min(move.from[0], move.to[0]);
        char endSqr = std::max(move.to[0], move.from[0]);
        Color opponent = playerInTurn == Color::WHITE ? Color::BLACK : Color::WHITE;
        for (char stepSquare = startSqr; stepSquare <= endSqr; stepSquare++)
        {
            if (isThreatened(stepSquare, opponent))
            {
                return false;
            }
        }
    }
    Board testBoard(*this);
    testBoard.applyMove(move);
    // Check if the current player in turn is in check if the move was applied.
    Piece currentPlayerColor = playerInTurn == Color::WHITE ? Piece::WHITE : Piece::BLACK;
    char kingSquare = testBoard.findSquareWithPiece(currentPlayerColor | Piece::KING);
    return (!testBoard.isThreatened(kingSquare, testBoard.playerInTurn));
}

bool Board::areSameColor(Piece p1, Piece p2)
{
    return !!(p1 & p2 & Piece::COLOR_MASK);
}

char Board::stepSquareInDirection(char square, MoveDirection direction)
{
    // Direction offsets are like this:
    // 7  8  9
    //-1  0  1
    //-9 -8 -7
    char file = square % 8;
    char rank = square / 8;
    // Would go through the left edge.
    if (file == 0 && (char(direction) + 1) % 8 == 0)
        return -1;

    // Would go through the left edge.
    if (file == 7 && (char(direction) - 1) % 8 == 0)
        return -1;

    // Check the bottom edge.
    if (rank == 0 && char(direction) < -1)
        return -1;

    // Check the top edge.
    if (rank == 7 && char(direction) > 1)
        return -1;

    return square + char(direction);
}

void Board::findPseudoPawnMoves(char square, Color player, std::vector<Move>& moves, bool onlyTakes, bool forceIncludeTakes) const
{
    MoveDirection pawnDirection = player == Color::WHITE ? MoveDirection::N : MoveDirection::S;
    char nextSquare = stepSquareInDirection(square, pawnDirection);
    bool promotion = nextSquare < 8 || nextSquare >= 7 * 8;

    assert(square < 7 * 8 && square >= 8 && "Pawn is never on the last rank.");

    if (!onlyTakes)
    {
        if (pieces[nextSquare] == Piece::NONE)
        {
            if (promotion)
            {
                moves.push_back(Move(square, nextSquare, Piece::QUEEN));
                moves.push_back(Move(square, nextSquare, Piece::KNIGHT));
                moves.push_back(Move(square, nextSquare, Piece::ROOK));
                moves.push_back(Move(square, nextSquare, Piece::BISHOP));
            }
            else 
            {
                moves.push_back(Move(square, nextSquare));
            }
            // Double step for a pawn?
            char rank = square / 8;
            char startRank = player == Color::WHITE ? 1 : 6;
            if (rank == startRank)
            {
                char nextnextSquare = stepSquareInDirection(nextSquare, pawnDirection);
                if (pieces[nextnextSquare] == Piece::NONE)
                    moves.push_back(Move(square, nextnextSquare));
            }
        }
    }
    MoveDirection attackDirections[2];
    attackDirections[0] = player == Color::WHITE ? MoveDirection::NW : MoveDirection :: SW;
    attackDirections[1] = player == Color::WHITE ? MoveDirection::NE : MoveDirection :: SE;
    for (int i = 0; i < 2; i++)
    {
        MoveDirection dir = attackDirections[i];
        char attackSquare = stepSquareInDirection(square, dir);
        if (attackSquare == -1)
            continue;

        if (enPassant == attackSquare)
        {
            char takePieceSquare = attackSquare - (char)pawnDirection;
            moves.push_back(Move(square, attackSquare, takePieceSquare, -1));
        }
        else if (forceIncludeTakes || (pieces[attackSquare] != Piece::NONE && !areSameColor(pieces[square], pieces[attackSquare])))
        {
            if (promotion)
            {
                moves.push_back(Move(square, attackSquare, Piece::QUEEN));
                moves.push_back(Move(square, attackSquare, Piece::KNIGHT));
                moves.push_back(Move(square, attackSquare, Piece::ROOK));
                moves.push_back(Move(square, attackSquare, Piece::BISHOP));
            }
            else 
            {
                moves.push_back(Move(square, attackSquare));
            }
        }
    }   
}

void Board::findDirectionalPseudoMoves(char square, const std::vector<MoveDirection>& directions, std::vector<Move>& moves, int maxSteps) const
{
    for (MoveDirection dir : directions) 
    {
        char nextSquare = stepSquareInDirection(square, dir);
        int stepsLeft = maxSteps;
        while (nextSquare != -1 && stepsLeft-- > 0)
        {
            if (pieces[nextSquare] == Piece::NONE) 
            {
                moves.push_back(Move(square, nextSquare));
                nextSquare = stepSquareInDirection(nextSquare, dir);
                continue;
            }
            if (!areSameColor(pieces[square], pieces[nextSquare]))
            {
                moves.push_back(Move(square, nextSquare));
            }
            break;
        }
    }
}

void Board::findPseudoRookMoves(char square, std::vector<Move>& moves) const
{
    std::vector<MoveDirection> directions {
        MoveDirection::N,
        MoveDirection::S,
        MoveDirection::E,
        MoveDirection::W
    };
    findDirectionalPseudoMoves(square, directions, moves);
}

void Board::findPseudoQueenMoves(char square, std::vector<Move>& moves) const
{
    std::vector<MoveDirection> directions {
        MoveDirection::N,
        MoveDirection::S,
        MoveDirection::E,
        MoveDirection::W,
        MoveDirection::NE,
        MoveDirection::SE,
        MoveDirection::SW,
        MoveDirection::NW
    };
    findDirectionalPseudoMoves(square, directions, moves);
}

void Board::findPseudoCastlingMoves(char square, Color player, std::vector<Move>& moves) const
{
    char rank = player == Color::WHITE ? 0 : 7;
    bool kingSideAvailable = player == Color::WHITE ? whiteCanCastleKing : blackCanCastleKing;
    bool queenSideAvailable = player == Color::WHITE ? whiteCanCastleQueen : blackCanCastleQueen;
    
    if (kingSideAvailable)
    {   
        char rookSquare = char(rank * 8 + kingRookFile);
        char leftEnd = std::min(square, char(rank * 8 + 5));
        char rightEnd = std::max(char(rank * 8 + 6), rookSquare);

        bool kingCastleFree = true;
        for (char stepSquare = leftEnd; stepSquare <= rightEnd; stepSquare++)
        {
            if (stepSquare == square || stepSquare == rookSquare)
                continue;
                
            if (pieces[stepSquare] != Piece::NONE)
            {
                kingCastleFree = false;
                break;
            }
        }
        if (kingCastleFree)
            moves.push_back(Move(square, char(rank * 8 + 6), rookSquare, char(rank * 8 + 5)));
    }

    if (queenSideAvailable)
    {
        char rookSquare = char(rank * 8 + queenRookFile);
        char rightEnd = std::max(square, char(rank * 8 + 3));
        char leftEnd = std::min(char(rank * 8 + 2), rookSquare);

        bool queenCastleFree = true;
        for (char stepSquare = leftEnd; stepSquare <= rightEnd; stepSquare++)
        {
            if (stepSquare == square || stepSquare == rookSquare)
                continue;
                
            if (pieces[stepSquare] != Piece::NONE)
            {
                queenCastleFree = false;
                break;
            }
        }
        if (queenCastleFree)
            moves.push_back(Move(square, char(rank * 8 + 2), rookSquare, char(rank * 8 + 3)));
    }
}

void Board::findPseudoKingMoves(char square, Color player, std::vector<Move>& moves, bool includeCastling) const
{
    std::vector<MoveDirection> directions {
        MoveDirection::N,
        MoveDirection::S,
        MoveDirection::E,
        MoveDirection::W,
        MoveDirection::NE,
        MoveDirection::SE,
        MoveDirection::SW,
        MoveDirection::NW
    };
    findDirectionalPseudoMoves(square, directions, moves, 1);
    if (includeCastling)
    {
        findPseudoCastlingMoves(square, player, moves);
    }
}

void Board::findPseudoBishopMoves(char square, std::vector<Move>& moves) const 
{
    std::vector<MoveDirection> directions {
        MoveDirection::NE,
        MoveDirection::SE,
        MoveDirection::SW,
        MoveDirection::NW
    };
    findDirectionalPseudoMoves(square, directions, moves);    
}

void Board::findPseudoKnightMoves(char square, std::vector<Move>& moves) const
{
    char file = square % 8;
    char rank = square / 8;
    char rankOffsets[8] = {  1, 2,2,1,-1,-2,-2,-1 };
    char fileOffsets[8] = { -2,-1,1,2, 2, 1,-1,-2 };
    for (int i = 0; i < 8; i++)
    {
        char moveRank = rank + rankOffsets[i];
        char moveFile = file + fileOffsets[i];
        if (moveRank > 7 || moveRank < 0 || moveFile < 0 || moveFile > 7)
            continue;
        
        char moveSquare = 8 * moveRank + moveFile;
        Piece targetSquarePiece = pieces[moveSquare];
        if (targetSquarePiece == Piece::NONE || !areSameColor(pieces[square], targetSquarePiece))
            moves.push_back(Move(square, moveSquare));
    }
}

void Board::findPseudoLegalMoves(char square, Color forPlayer, std::vector<Move>& pseudoLegalMoves, bool pawnOnlyTakes, bool forceIncludePawnTakes) const
{
    PROFILE("Board::findPseudoLegalMoves");
    Piece piece = pieces[square];
    Piece currentPlayer = forPlayer == Color::WHITE ? Piece::WHITE : Piece::BLACK;
    if (!(piece & currentPlayer))
    {
        return;
    }
    Piece pieceType = piece & ~Piece::COLOR_MASK;
    
    switch (pieceType)
    {
    case Piece::PAWN:
        return findPseudoPawnMoves(square, forPlayer, pseudoLegalMoves, pawnOnlyTakes, forceIncludePawnTakes);
    case Piece::ROOK:
        return findPseudoRookMoves(square, pseudoLegalMoves);
    case Piece::QUEEN:
        return findPseudoQueenMoves(square, pseudoLegalMoves);
    case Piece::KING:
        return findPseudoKingMoves(square, forPlayer, pseudoLegalMoves);
    case Piece::BISHOP:
        return findPseudoBishopMoves(square, pseudoLegalMoves);
    case Piece::KNIGHT:
        return findPseudoKnightMoves(square, pseudoLegalMoves);
    default:
        break;
    }
}

bool Board::isThreatened(char square, Color byPlayer) const
{
    // PROFILE("Board::isThreatened");
    // Threats are symmetric: if a knight in this square would threaten a knight of the other player,
    // the other knight would also threaten this square.
    // Hence, for each piece typee check if such piece in this square would threaten a similar piece of the other player.
    // If that's true for any piece type, then this square is threatened.
    Piece testPlayerColor = byPlayer == Color::BLACK ? Piece::WHITE : Piece::BLACK;
    Piece opponentColor = ~testPlayerColor & Piece::COLOR_MASK;
    
    // Test for knight threat
    char file = square % 8;
    char rank = square / 8;
    // Offsets (compared to the current square) for the 8 possible squares a knight can threat this square from
    char rankOffsets[8] = {  1, 2,2,1,-1,-2,-2,-1 };
    char fileOffsets[8] = { -2,-1,1,2, 2, 1,-1,-2 };
    // Check each of the 8 squares if there is an enemy knight there
    for (int i = 0; i < 8; i++)
    {
        char moveRank = rank + rankOffsets[i];
        char moveFile = file + fileOffsets[i];
        if (moveRank > 7 || moveRank < 0 || moveFile < 0 || moveFile > 7)
            continue;
        
        char moveSquare = 8 * moveRank + moveFile;
        Piece targetSquarePiece = pieces[moveSquare];
        if (targetSquarePiece == (Piece::KNIGHT | opponentColor))
            return true;
    }

    // Test for pawn threat
    if (hasPawnThreat(square, byPlayer))
        return true;

    // Test for king threat
    MoveDirection kingDirections[8] = {
        MoveDirection::N,
        MoveDirection::S,
        MoveDirection::E,
        MoveDirection::W,
        MoveDirection::NE,
        MoveDirection::SE,
        MoveDirection::SW,
        MoveDirection::NW
    };
    for (int i = 0; i < 8; i++)
    {
        char threatSquare = stepSquareInDirection(square, kingDirections[i]);
        if (threatSquare != char(-1))
        {
            Piece threatPiece = pieces[threatSquare];
            if (threatPiece == (opponentColor | Piece::KING))
                return true;
        }
    }

    // Test for other threats, first vertical and then straight lines.
    MoveDirection testDirections[2][4] = {
        {
            MoveDirection::NE,
            MoveDirection::SE,
            MoveDirection::SW,
            MoveDirection::NW
        },
        {
            MoveDirection::N,
            MoveDirection::S,
            MoveDirection::E,
            MoveDirection::W
        }
    };
    Piece threatPieces[2] = {
        Piece::BISHOP | Piece::QUEEN,
        Piece::ROOK | Piece::QUEEN
    };

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            char nextSquare = stepSquareInDirection(square, testDirections[i][j]);
            while (nextSquare != -1)
            {
                Piece targetPiece = pieces[nextSquare];

                // Own pieces are not threats
                if (!!(targetPiece & testPlayerColor))
                    break;
                
                // The piece is opponent and can move here, it's a threat
                if (!!(targetPiece & threatPieces[i]))
                    return true;

                // The piece is opponent's but cannot move here, not a threat
                if (targetPiece != Piece::NONE)
                    break;

                nextSquare = stepSquareInDirection(nextSquare, testDirections[i][j]);
            }
        }
    }

    return false;
}

bool Board::hasPawnThreat(char square, Color byPlayer) const
{
    Piece byPlayerColor = byPlayer == Color::WHITE ? Piece::WHITE : Piece::BLACK;
    MoveDirection possibleThreats[2];
    possibleThreats[0] = byPlayerColor == Piece::BLACK ? MoveDirection::NE : MoveDirection::SE;
    possibleThreats[1] = byPlayerColor == Piece::BLACK ? MoveDirection::NW : MoveDirection::SW;
    for (int i = 0; i < 2; i++)
    {
        char threatSquare = stepSquareInDirection(square, possibleThreats[i]);
        if (threatSquare != char(-1))
        {
            Piece threatPiece = pieces[threatSquare];
            if (threatPiece == (byPlayerColor | Piece::PAWN))
                return true;
        }
    }
    return false;
}

Move Board::constructPromotionMove(const std::string& moveUCI) const
{
    const char* moveStr = moveUCI.c_str();
    const char firstSquareIdx = BoardFuncs::getSquareIndex(moveStr);
    const char secondSquareIdx = BoardFuncs::getSquareIndex(&moveStr[2]);
    Move move(firstSquareIdx, secondSquareIdx);
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

Move Board::constructCastlingMove(char firstSquare, char secondSquare) const
{
    char rank = playerInTurn == Color::WHITE ? 0 : 7;
    char rookFile = firstSquare > secondSquare ? queenRookFile : kingRookFile;
    char rookSquare = 8 * rank + rookFile;
    char kingTargetFile = firstSquare < secondSquare ? 6 : 2;
    char rookTargetFile = firstSquare < secondSquare ? 5 : 3;
    Move move(firstSquare, 8 * rank + kingTargetFile, rookSquare, 8 * rank + rookTargetFile);
    return move;
}

Move Board::constructEnPassantMove(char firstSquare, char secondSquare) const
{
    char takeSquare = secondSquare + (playerInTurn == Color::WHITE ? -8 : 8);
    Move move(firstSquare, secondSquare, takeSquare, -1);
    return move;
}

Move Board::constructMove(const std::string& moveUCI) const
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

    if (!!(firstSquareData & Piece::PAWN) && secondSquareIdx == enPassant)
    {
        return constructEnPassantMove(firstSquareIdx, secondSquareIdx);
    }

    Move move(firstSquareIdx, secondSquareIdx);
    return move;
}

void Board::applyMove(const std::string& moveUCI) 
{
    applyMove(constructMove(moveUCI));
}

void Board::applyMove(const Move& move) 
{
    PROFILE("Board::applyMove");
    // Remove the old en passant file from the hash
    if (enPassant != -1)
    {
        int enPassantFile = enPassant % 8;
        hash.toggleEnPassant(enPassantFile);
        enPassant = -1;
    }

    // In 960, the king might be moving where the rook is at the moment. For this reason,
    // we store both pieces before moving anything, to not lose the piece data.
    Piece movePieces[2];
    for (int i = 0; i < 2; i++)
    {
        char from = move.from[i];
        if (from < 0)
            continue;

        movePieces[i] = pieces[from];
    }

    for (int i = 0; i < 2; i++)
    {
        char from = move.from[i];
        char to = move.to[i];
        if (from < 0)
            continue;

        Piece movePiece = movePieces[i];

        // Update the Zobrist hash, remove the piece from the old square
        hash.togglePiece(from, movePiece);

        // Prepare potential promotion
        if (move.promotion != Piece::NONE)
        {
            // Keep the color but change the piece type.
            movePiece = (Piece::COLOR_MASK & movePiece) | move.promotion;
        }

        // Update en passant square
        if (!!(movePiece & Piece::PAWN))
        {
            // A pawn move clears the repetition history because they can never go back.
            resetRepetitionHistory();
            // Moved two ranks to either direction?
            if (std::abs(from - to) == (char)16)
            {
                // En passant square is between from and to.
                char enPassantCandidate = (from + to) / 2;
                if (hasPawnThreat(enPassantCandidate, playerInTurn == Color::WHITE ? Color::BLACK : Color::WHITE))
                {
                    // Add the new en passant file to the hash
                    enPassant = enPassantCandidate;
                    int enPassantFile = enPassantCandidate % 8;
                    hash.toggleEnPassant(enPassantFile);
                }
            }
        }
        pieces[from] = Piece::NONE;
        if (to >= 0 && to < 64)
        {
            // Remove the old piece from the target square if there was one
            Piece targetPiece = pieces[to];
            if (targetPiece != Piece::NONE)
            {
                // A capture is also irrevertible and clears the repetition history.
                resetRepetitionHistory();
                hash.togglePiece(to, targetPiece);
            }

            pieces[to] = movePiece;
            // Update the Zobrist hash, add the piece to the new square
            hash.togglePiece(to, movePiece);
        }
    }

    bool oldCastlingRights[4] = { whiteCanCastleKing, whiteCanCastleQueen, blackCanCastleKing, blackCanCastleQueen };

    updateCastlingRights();

    // Also forcefully update the castling rights if this move was a castling. Otherwise
    // weird scenarios are possible in 960 if the king stays still while castling.
    if (move.isCastling())
    {
        if (playerInTurn == Color::WHITE)
        {
            whiteCanCastleKing = false;
            whiteCanCastleQueen = false;
        }
        else 
        {
            blackCanCastleKing = false;
            blackCanCastleQueen = false;
        }
    }

    bool newCastlingRights[4] = { whiteCanCastleKing, whiteCanCastleQueen, blackCanCastleKing, blackCanCastleQueen };

    // Update the hash with the new castling rights
    if (oldCastlingRights[0] != newCastlingRights[0])
    {
        hash.toggleCastlingRights(Color::WHITE, 'k');
    }
    if (oldCastlingRights[1] != newCastlingRights[1])
    {
        hash.toggleCastlingRights(Color::WHITE, 'q');
    }
    if (oldCastlingRights[2] != newCastlingRights[2])
    {
        hash.toggleCastlingRights(Color::BLACK, 'k');
    }
    if (oldCastlingRights[3] != newCastlingRights[3])
    {
        hash.toggleCastlingRights(Color::BLACK, 'q');
    }

    playerInTurn = playerInTurn == Color::BLACK ? Color::WHITE : Color::BLACK;
    hash.togglePlayerInTurn();
    updateRepetitionHistory();
}

void Board::updateCastlingRights()
{
    // Update for white
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
    {
        if (blackCanCastleQueen)
        {
            char qRookOrigSquare = 8 * 7 + queenRookFile;
            if (pieces[qRookOrigSquare] != (Piece::BLACK | Piece::ROOK))
                blackCanCastleQueen = false;
        }
        if (blackCanCastleKing)
        {
            char kRookOrigSquare = 8 * 7 + kingRookFile;
            if (pieces[kRookOrigSquare] != (Piece::BLACK | Piece::ROOK))
                blackCanCastleKing = false;
        }
        if (blackCanCastleKing || blackCanCastleQueen)
        {
            char origKingSquare = 8 * 7 + kingStartFile;
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

Piece Board::getSquare(char sqr) const
{
    return pieces[sqr];
}

Piece Board::getSquare(const char* sqr) const
{
    return pieces[BoardFuncs::getSquareIndex(sqr)];
}

Piece Board::getSquare(char file, char rank) const
{
    return pieces[8 * rank + file];
}

Color Board::getCurrentPlayer() const
{
    return playerInTurn;
}

unsigned char Board::turnsSincePawnMoveOrCapture() const
{
    if (whitePositionsSize == 0u && blackPositionsSize == 0u)
    {
        return 0u;
    }
    return std::max(whitePositionsSize, blackPositionsSize) - 1;
}

bool Board::isCheck() const
{
    Color opponentColor = playerInTurn == Color::WHITE ? Color::BLACK : Color::WHITE;
    Piece currentPlayerColor = playerInTurn == Color::WHITE ? Piece::WHITE : Piece::BLACK;
    char kingSquare = findSquareWithPiece(currentPlayerColor | Piece::KING);
    return isThreatened(kingSquare, opponentColor);
}

bool Board::isMate() const
{
    return isCheck() && findPossibleMoves().size() == 0;
}

bool Board::insufficientMaterial() const
{
    bool darkSquareBishop = false;
    bool lightSquareBishop = false;
    bool knightOnBoard = false;
    for (char square = 0; square < 64; square++)
    {
        // If there is a pawn, queen or rook on the board, it is not insufficient material
        if (!!(pieces[square] & Piece::PAWN))
            return false;
        if (!!(pieces[square] & Piece::QUEEN))
            return false;
        if (!!(pieces[square] & Piece::ROOK))
            return false;

        // If there are more than one knight on board, it's not insufficient material
        if (!!(pieces[square] & Piece::KNIGHT))
        {
            if (knightOnBoard)
                return false;
            knightOnBoard = true;
        }

        // If there are bishops on both square colors, it's not insufficient material
        if (!!(pieces[square] & Piece::BISHOP))
        {
            char rank = square / 8;
            bool lightSquare = square % 2 + rank % 2 == 1;
            if (lightSquare)
            {
                if (darkSquareBishop)
                    return false;
                lightSquareBishop = true;
            }
            else
            {
                if (lightSquareBishop)
                    return false;
                darkSquareBishop = true;
            }
        }

        // Bishop and knight is also enough for a checkmate.
        if ((darkSquareBishop | lightSquareBishop) && knightOnBoard)
            return false;
    }
    return true;
}

bool Board::noProgress() const 
{
    return turnsSincePawnMoveOrCapture() >= 50u;
}

bool Board::threefoldRepetition() const 
{
    return highestRepetitionCount >= 3u;
}   

void Board::updateRepetitionHistory()
{
    unsigned char repeatCount = 1u;
    if (playerInTurn == Color::WHITE)
    {
        for (int i = 0; i < whitePositionsSize; i++)
        {
            if (repeatablePositionsWhite[i] == hash.getHash())
            {
                repeatCount++;
            }
        }
        repeatablePositionsWhite[whitePositionsSize++] = hash.getHash();
    }
    else 
    {
        for (int i = 0; i < blackPositionsSize; i++)
        {
            if (repeatablePositionsBlack[i] == hash.getHash())
            {
                repeatCount++;
            }
        }
        repeatablePositionsBlack[blackPositionsSize++] = hash.getHash();
    }
    highestRepetitionCount = std::max(highestRepetitionCount, repeatCount);
}

void Board::resetRepetitionHistory()
{
    highestRepetitionCount = 0u;
    whitePositionsSize = 0u;
    blackPositionsSize = 0u;
}

unsigned int Board::getHash()
{
    return hash.getHash();
}
