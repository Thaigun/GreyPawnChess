#include "MonteCarloNode.h"

#include <cmath>
#include <limits>
#include <vector>

#include "Random.h"

Color MonteCarloNode::runIteration(Board& board, unsigned int totalSimCount, unsigned int maxMoveCount)
{
    Color nodePlayerColor = board.getCurrentPlayer();
    if (!isLeaf())
    {
        nodeIterations++;
        Move bestMove;
        MonteCarloNode* bestChild = highestUCB1Child(totalSimCount, board.getCurrentPlayer(), &bestMove);
        board.applyMove(bestMove);
        Color winner = bestChild->runIteration(board, totalSimCount, maxMoveCount - 1);
        if (winner == nodePlayerColor)
            points += 1.0f;
        else if (winner == Color::NONE)
            points += 0.5f;

        return winner;
    }

    // Is leaf
    // If the node is already end of game, return immediately.
    std::vector<Move> initialMoves = board.findPossibleMoves();
    if (initialMoves.size() == 0)
    {
        nodeIterations++;
        Color winner = Color::NONE;
        if (board.isCheck())
        {
            // Current player is in mate.
            winner = board.getCurrentPlayer() == Color::BLACK ? Color::WHITE : Color::BLACK;
        }
        return winner;
    }

    // Node has not been simulated yet.
    if (nodeIterations++ == 0u)
    {
        unsigned int movesLeft = maxMoveCount;
        Color simulationWinner = Color::NONE;
        
        while (movesLeft-- > 0u)
        {
            std::vector<Move> nextMoves = board.findPossibleMoves();
            if (nextMoves.size() == 0)
            {
                if (board.isCheck())
                {
                    // Mate
                    simulationWinner = board.getCurrentPlayer() == Color::BLACK ? Color::WHITE : Color::BLACK;
                    break;
                }
                simulationWinner = Color::NONE;
                break;
            }
            int moveIdx = Random::Range(0, (int)nextMoves.size() - 1);
            board.applyMove(nextMoves[moveIdx]);
        }

        if (nodePlayerColor == simulationWinner)
            points += 1.0f;
        else if (simulationWinner == Color::NONE)
            points += 0.5f;

        return simulationWinner;
    }

    // Expand the node if it has been simulated and did not have children yet.
    possibleMoves = std::move(initialMoves);
    childNodes.resize(possibleMoves.size());
    board.applyMove(possibleMoves[0]);
    return childNodes[0].runIteration(board, totalSimCount, maxMoveCount - 1);
}

bool MonteCarloNode::isLeaf()
{
    return !childNodes.size();
}

float MonteCarloNode::UCB1(unsigned int totalVisits, Color player)
{
    if (!nodeIterations)
    {
        return FLT_MAX;
    }

    float exploitationFactor = points / nodeIterations;
    float explorationFactor = 2 * float(std::sqrt(std::log(totalVisits) / nodeIterations));
    return exploitationFactor + explorationFactor;
}

MonteCarloNode* MonteCarloNode::highestUCB1Child(unsigned int totalVisits, Color player, const Move* populateMove)
{
    float bestChildUCB1 = FLT_MIN;
    MonteCarloNode* bestChild = nullptr;
    for (int i = 0; i < childNodes.size(); i++)
    {
        float thisChildUCB1 = childNodes[i].UCB1(totalVisits, player);
        if (thisChildUCB1 > bestChildUCB1)
        {
            populateMove = &possibleMoves[i];
            bestChild = &childNodes[i];
            if (thisChildUCB1 == FLT_MAX)
                return bestChild;
        }
    }
    return bestChild;
}

unsigned int MonteCarloNode::nodeVisits() const
{
    return nodeIterations;
}

Move MonteCarloNode::highestWinrateMove() const
{
    float bestWinRate = FLT_MIN;
    Move bestMove;
    for (int i = 0; i < childNodes.size(); i++)
    {
        float childWinrate = childNodes[i].points / childNodes[i].nodeIterations;
        if (childWinrate > bestWinRate)
        {
            bestWinRate = childWinrate;
            bestMove = possibleMoves[i];
        }
    }
    return bestMove;
}

MonteCarloNode& MonteCarloNode::getNodeForMove(const Move& move)
{
    // If this node has not been expanded yet, just return a new node.
    if (isLeaf())
    {
        return MonteCarloNode();
    }

    for (int i = 0; i < childNodes.size(); i++)
    {
        if (possibleMoves[i] == move)
        {
            return childNodes[i];
        }
    }

    // If, for some reason matching node is not found, return a new, empty node.
    return MonteCarloNode();
}