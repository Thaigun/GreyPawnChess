#include "MonteCarloTree.h"

#include "Board.h"

void MonteCarloTree::runIteration()
{
    Board simulationBoard;
    MonteCarloNode& currentNode = root;
    currentNode.runIteration(simulationBoard, root.nodeVisits());
}

Move MonteCarloTree::getBestOption()
{
    return root.highestWinrateMove();
}

void MonteCarloTree::applyMove(const Move& move)
{
    
}
