#include "Board.h"

#include <algorithm>
#include <assert.h>
#include <utility>

#include "BoardFuncs.h"

std::vector<Move> Board::findPossibleMoves()
{
    std::vector<Move> moves;
    for (char sqr = 0; sqr < (char)64; sqr++)
    {
        findLegalMovesForSquare(sqr, moves);
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
    std::vector<Move> pseudoMoves = findPseudoLegalMoves(square, playerInTurn);
    for (Move move : pseudoMoves)
    {
        Board testBoard(*this);
        
        if (move.isCastling()) 
        {
            char startSqr = std::min(move.from[0], move.to[0]);
            char endSqr = std::max(move.to[0], move.from[0]);
            bool castlingThreatened = false;
            Color opponent = playerInTurn == Color::WHITE ? Color::BLACK : Color::WHITE;
            for (char square = startSqr; square <= endSqr; square++)
            {
                if (isThreatened(square, opponent))
                {
                    castlingThreatened = true;
                    break;
                }
            }
            if (castlingThreatened)
                continue;
        }

        testBoard.applyMove(move);
        // Check if the current player in turn is in check if the move was applied.
        Piece currentPlayerColor = playerInTurn == Color::WHITE ? Piece::WHITE : Piece::BLACK;
        char kingSquare = testBoard.findSquareWithPiece(currentPlayerColor | Piece::KING);
        if (!testBoard.isThreatened(kingSquare, testBoard.playerInTurn))
        {
            moveList.push_back(move);
        }
    }
}

bool Board::areSameColor(Piece p1, Piece p2)
{
    return !!(p1 & p2 & Piece::COLOR_MASK);
}

char Board::stepSquareInDirection(char square, MoveDirection direction)
{
    // Direction offsets are like this:
    // 7  8  9
    //-1  1  1
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

std::vector<Move> Board::findPseudoPawnMoves(char square, Color player, bool onlyTakes) const
{
    char file = square % 8;
    char rank = square / 8;
    char nextRank = player == Color::WHITE ? rank + 1 : rank - 1;
    bool promotion = nextRank == 7 || nextRank == 0;
    std::vector<Move> moves;

    assert(square < 7 * 8 && square >= 8 && "Pawn is never on the last rank.");

    if (!onlyTakes && getSquare(file, nextRank) == Piece::NONE)
    {
        char nextSquareIdx = nextRank * 8 + file;
        if (promotion)
        {
            moves.push_back(Move(square, nextSquareIdx, Piece::QUEEN));
            moves.push_back(Move(square, nextSquareIdx, Piece::KNIGHT));
            moves.push_back(Move(square, nextSquareIdx, Piece::ROOK));
            moves.push_back(Move(square, nextSquareIdx, Piece::BISHOP));
        }
        else 
        {
            moves.push_back(Move(square, nextSquareIdx));
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
        if (enPassant == attackSquare || 
            (pieces[attackSquare] != Piece::NONE && !areSameColor(pieces[square], pieces[attackSquare])))
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
    return moves; 
}

std::vector<Move> Board::findDirectionalPseudoMoves(char square, const std::vector<MoveDirection>& directions, int maxSteps) const
{
    int stepsLeft = maxSteps;
    std::vector<Move> moves;
    for (MoveDirection dir : directions) 
    {
        char nextSquare = stepSquareInDirection(square, dir);
        while (nextSquare != -1 && maxSteps-- > 0)
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
    return moves;
}

std::vector<Move> Board::findPseudoRookMoves(char square, Color player) const
{
    std::vector<MoveDirection> directions {
        MoveDirection::N,
        MoveDirection::S,
        MoveDirection::E,
        MoveDirection::W
    };
    return findDirectionalPseudoMoves(square, directions);
}

std::vector<Move> Board::findPseudoQueenMoves(char square, Color player) const
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
    return findDirectionalPseudoMoves(square, directions);
}

std::vector<Move> Board::findPseudoCastlingMoves(char square, Color player) const
{
    std::vector<Move> moves;
    char rank = player == Color::WHITE ? 0 : 7;
    bool kingSideAvailable = player == Color::WHITE ? whiteCanCastleKing : blackCanCastleKing;
    bool queenSideAvailable = player == Color::WHITE ? whiteCanCastleQueen : blackCanCastleQueen;
    char currentFile = square % 8;
    
    if (kingSideAvailable)
    {   
        char rookSquare = char(rank * 8 + kingRookFile);
        char leftEnd = std::min(square, char(rank * 8 + 5));
        char rightEnd = std::max(char(rank * 8 + 6), rookSquare);

        bool kingCastleFree = true;
        for (char stepSquare = leftEnd; square <= rightEnd; stepSquare++)
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

        int kingsAllowed = 1;
        int rooksAllowed = 1;
        bool queenCastleFree = true;
        for (char stepSquare = leftEnd; square <= rightEnd; stepSquare++)
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

std::vector<Move> Board::findPseudoKingMoves(char square, Color player) const
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
    std::vector<Move> moves = findDirectionalPseudoMoves(square, directions, 1);
    std::vector<Move> castlingMoves = findPseudoCastlingMoves(square, player);
    moves.insert(moves.end(), castlingMoves.begin(), castlingMoves.end());
    return moves;
}

std::vector<Move> Board::findPseudoBishopMoves(char square, Color player) const 
{
    std::vector<MoveDirection> directions {
        MoveDirection::NE,
        MoveDirection::SE,
        MoveDirection::SW,
        MoveDirection::NW
    };
    return findDirectionalPseudoMoves(square, directions);    
}

std::vector<Move> Board::findPseudoKnightMoves(char square, Color player) const
{
    std::vector<Move> moves;
    char rank = square % 8;
    char file = square / 8;
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
        if (targetSquarePiece == Piece::NONE || areSameColor(pieces[square], targetSquarePiece))
            moves.push_back(Move(square, moveSquare));
    }
}

std::vector<Move> Board::findPseudoLegalMoves(char square, Color forPlayer, bool pawnOnlyTakes) const
{
    std::vector<Move> moves;
    Piece piece = pieces[square];
    Piece currentPlayer = forPlayer == Color::WHITE ? Piece::WHITE : Piece::BLACK;
    if (!(piece & currentPlayer))
    {
        return moves;
    }
    
    if (!!(piece & Piece::PAWN))
    {
        return findPseudoPawnMoves(square, playerInTurn, pawnOnlyTakes);
    }
    else if (!!(piece & Piece::ROOK))
    {
        return findPseudoRookMoves(square, playerInTurn);
    }
    else if (!!(piece & Piece::QUEEN))
    {
        return findPseudoQueenMoves(square, playerInTurn);
    }
    else if (!!(piece & Piece::KING))
    {
        return findPseudoKingMoves(square, playerInTurn);
    }
    else if (!!(piece & Piece::BISHOP))
    {
        return findPseudoBishopMoves(square, playerInTurn);
    }
    else if (!!(piece & Piece::KNIGHT))
    {
        return findPseudoKnightMoves(square, playerInTurn);
    }
    return moves;
}

bool Board::isThreatened(char square, Color byPlayer) const
{
    Piece targetPlayerColor = byPlayer == Color::BLACK ? Piece::WHITE : Piece::BLACK;
    // Find pseudo moves for all opponent pieces, if the king is in one of them, is check.
    for (char i = 0; i < 64; i++)
    {
        std::vector<Move> moves = findPseudoLegalMoves(i, byPlayer, true);
        for (const Move& move : moves)
        {
            if (move.to[0] == square)
                return true;
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
    // Reset, it will be set again if needed
    enPassant = -1;

    // In 960, the king might be moving where the rook is at the moment. For this reason,
    // we store both pieces before moving anything, to not lose the piece data.
    Piece movePieces[2];
    for (int i = 0; i < 2; i++)
    {
        char from = move.from[i];
        char to = move.to[i];
        if (from < 0 || to < 0)
            continue;

        movePieces[i] = pieces[from];
    }

    for (int i = 0; i < 2; i++)
    {
        char from = move.from[i];
        char to = move.to[i];
        if (from < 0 || to < 0)
            continue;

        Piece movePiece = movePieces[i];
        // Prepare potential promotion
        if (move.promotion != Piece::NONE)
        {
            // Keep the color but change the piece type.
            movePiece = (Piece::COLOR_MASK & movePiece) | move.promotion;
        }

        // Update en passant square
        if (!!(movePiece & Piece::PAWN))
        {
            // Moved to ranks to either direction?
            if (std::abs(from - to) == (char)16)
            {
                // En passant square is between from and to.
                enPassant = (from + to) / 2;
            }
        }
        pieces[from] = Piece::NONE;
        pieces[to] = movePiece;
    }

    updateCastlingRights(playerInTurn);

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

    // Update whose turn it is
    playerInTurn = playerInTurn == Color::BLACK ? Color::WHITE : Color::BLACK;
}

void Board::updateCastlingRights(Color color)
{
    // Update for white
    if (color == Color::WHITE)
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
    if (color == Color::BLACK)
    {
        if (blackCanCastleQueen)
        {
            char qRookOrigSquare = queenRookFile;
            if (pieces[qRookOrigSquare] != (Piece::BLACK | Piece::ROOK))
                blackCanCastleQueen = false;
        }
        if (blackCanCastleKing)
        {
            char kRookOrigSquare = kingRookFile;
            if (pieces[kRookOrigSquare] != (Piece::BLACK | Piece::ROOK))
                blackCanCastleKing = false;
        }
        if (blackCanCastleKing || blackCanCastleQueen)
        {
            char origKingSquare = kingStartFile;
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