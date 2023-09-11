#include "RandomStrategy.h"
#include "../Random.h"

void RandomStrategy::tickComputation()
{
    confidence = 1.0f;
}

void RandomStrategy::applyMoveToStrategy(const Move& move)
{
}

Move RandomStrategy::getBestMove()
{
    std::vector<Move> moves = board.findPossibleMoves();
    return moves[Random::Range(0, moves.size() - 1)];
}