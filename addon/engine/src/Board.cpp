#include "Board.h"

#include <algorithm>
#include <assert.h>
#include <climits>
#include <iterator>
#include <sstream>
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

    initHash();
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
    newBoard.initHash();
    return newBoard;
}

std::vector<Move> Board::findPossibleMoves() const
{
    PROFILE("Board::findPossibleMoves");
    std::vector<Move> pseudoMoves;
    for (char sqr = 0; sqr < (char)64; sqr++)
    {
        findPseudoLegalMoves(sqr, playerInTurn, pseudoMoves, false, false);
    }

    std::vector<Move> moves;
    for (const Move& move : pseudoMoves)
    {
        if (checkMoveLegality(move))
        {
            moves.push_back(move);
        }
    }

    return moves;
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

void Board::findPseudoKingMoves(char square, Color player, std::vector<Move>& moves) const
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
    findPseudoCastlingMoves(square, player, moves);
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
    
    if (!!(piece & Piece::PAWN))
    {
        return findPseudoPawnMoves(square, forPlayer, pseudoLegalMoves, pawnOnlyTakes, forceIncludePawnTakes);
    }
    else if (!!(piece & Piece::ROOK))
    {
        return findPseudoRookMoves(square, pseudoLegalMoves);
    }
    else if (!!(piece & Piece::QUEEN))
    {
        return findPseudoQueenMoves(square, pseudoLegalMoves);
    }
    else if (!!(piece & Piece::KING))
    {
        return findPseudoKingMoves(square, forPlayer, pseudoLegalMoves);
    }
    else if (!!(piece & Piece::BISHOP))
    {
        return findPseudoBishopMoves(square, pseudoLegalMoves);
    }
    else if (!!(piece & Piece::KNIGHT))
    {
        return findPseudoKnightMoves(square, pseudoLegalMoves);
    }
}

bool Board::isThreatened(char square, Color byPlayer) const
{
    PROFILE("Board::isThreatened");
    // Threats are symmetric: if a knight in this square would threaten a knight of the other player,
    // the other knight would also threaten this square.
    // Hence, for each piece typewe check if such piece in this square would threaten a similar piece of the other player.
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
    MoveDirection possibleThreats[2];
    possibleThreats[0] = testPlayerColor == Piece::WHITE ? MoveDirection::NE : MoveDirection::SE;
    possibleThreats[1] = testPlayerColor == Piece::WHITE ? MoveDirection::NW : MoveDirection::SW;
    for (int i = 0; i < 2; i++)
    {
        char threatSquare = stepSquareInDirection(square, possibleThreats[i]);
        if (threatSquare != char(-1))
        {
            Piece threatPiece = pieces[threatSquare];
            if (threatPiece == (opponentColor | Piece::PAWN))
                return true;
        }
    }

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

Move Board::constructPromotionMove(const std::string& moveUCI)
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

Move Board::constructCastlingMove(char firstSquare, char secondSquare)
{
    char rank = playerInTurn == Color::WHITE ? 0 : 7;
    char rookFile = firstSquare > secondSquare ? queenRookFile : kingRookFile;
    char rookSquare = 8 * rank + rookFile;
    char kingTargetFile = firstSquare < secondSquare ? 6 : 2;
    char rookTargetFile = firstSquare < secondSquare ? 5 : 3;
    Move move(firstSquare, 8 * rank + kingTargetFile, rookSquare, 8 * rank + rookTargetFile);
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
    Move move(firstSquareIdx, secondSquareIdx);
    return move;
}

void Board::applyMove(const Move& move) 
{
    // Remove the old en passant file from the hash
    if (enPassant != -1)
    {
        int enPassantFile = enPassant % 8;
        hash ^= getZobristHashTable()[12 * 64 + 1 + 4 + enPassantFile];
    }

    // Reset, it will be set again if needed
    enPassant = -1;

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
        hash ^= getZobristHashTable()[from * 12 + zobristPieceKey(movePiece)];

        // Prepare potential promotion
        if (move.promotion != Piece::NONE)
        {
            // Keep the color but change the piece type.
            movePiece = (Piece::COLOR_MASK & movePiece) | move.promotion;
        }

        // Update en passant square
        if (!!(movePiece & Piece::PAWN))
        {
            // Moved two ranks to either direction?
            if (std::abs(from - to) == (char)16)
            {
                // En passant square is between from and to.
                enPassant = (from + to) / 2;
                // Add the new en passant file to the hash
                int enPassantFile = enPassant % 8;
                hash ^= getZobristHashTable()[12 * 64 + 1 + 4 + enPassantFile];
            }
        }
        pieces[from] = Piece::NONE;
        if (to >= 0 && to < 64)
        {
            // Remove the old piece from the target square if there was one
            Piece targetPiece = pieces[to];
            if (targetPiece != Piece::NONE)
            {
                hash ^= getZobristHashTable()[to * 12 + zobristPieceKey(targetPiece)];
            }

            pieces[to] = movePiece;
            // Update the Zobrist hash, add the piece to the new square
            hash ^= getZobristHashTable()[to * 12 + zobristPieceKey(movePiece)];
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
    for (int i = 0; i < 4; i++)
    {
        if (oldCastlingRights[i] != newCastlingRights[i])
        {
            hash ^= getZobristHashTable()[12 * 64 + 1 + i];
        }
    }

    // Update whose turn it is
    playerInTurn = playerInTurn == Color::BLACK ? Color::WHITE : Color::BLACK;
    hash ^= getZobristHashTable()[12 * 64];
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

unsigned int Board::getHash()
{
    return hash;
}

void Board::initHash()
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

int Board::zobristPieceKey(Piece piece) 
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

unsigned int* Board::getZobristHashTable()
{
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