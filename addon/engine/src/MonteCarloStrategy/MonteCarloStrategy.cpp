#include "MonteCarloStrategy.h"

void MonteCarloStrategy::tickComputation()
{
    // Run a few iterations of the Monte Carlo search.
    for (int i = 0; i < 10; i++)
    {
        monteCarloTree.runIteration(board, monteCarloTree.nodeVisits());
    }

    // This must be set in this method if it's our turn.
    confidence = 0.5f;
}

void MonteCarloStrategy::applyMoveToStrategy(const Move& move)
{
    monteCarloTree = std::move(monteCarloTree.getNodeForMove(move));
}

Move MonteCarloStrategy::getBestMove()
{
    return monteCarloTree.highestWinrateMove();
}