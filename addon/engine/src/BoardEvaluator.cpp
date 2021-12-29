#include <unordered_map>

#include "Board.h"
#include "Piece.h"

namespace BoardEvaluator
{
    std::unordered_map<Piece, float> pieceValues = {
        { Piece::PAWN, 1.0f },
        { Piece::KNIGHT, 3.0f },
        { Piece::BISHOP, 3.0f },
        { Piece::ROOK, 5.0f },
        { Piece::QUEEN, 9.0f },
        { Piece::KING, 0.0f }
    };

    float evaluateBoard(const Board& board)
    {
        float evaluation = 0.0f;
        for (char sqr = 0; sqr < 64; sqr++)
        {
            Piece piece = board.getSquare(sqr);
            if (piece == Piece::NONE)
                continue;

            float pieceValue = pieceValues[piece & ~Piece::COLOR_MASK];
            if (!!(piece & Piece::BLACK))
                pieceValue *= -1.0f;
            evaluation += pieceValue;
        }
        return evaluation;
    }
}