#include "MonteCarloNode.h"

#include <assert.h>
#include <cmath>
#include <limits>
#include <vector>

#include "../Board.h"
#include "../BoardEvaluator.h"
#include "../Random.h"

float MonteCarloNode::runIteration(const Board& board, unsigned int totalSimCount, unsigned int maxMoveCount)
{
    Board iterationBoard = board;
    return runIterationOnBoard(iterationBoard, totalSimCount, maxMoveCount, true);
}

float MonteCarloNode::runIterationOnBoard(Board& board, unsigned int totalSimCount, unsigned int maxMoveCount, bool isRoot)
{
    Color nodePlayerColor = board.getCurrentPlayer();
    if (!isLeaf())
    {
        nodeIterations++;
        Move bestMove;
        MonteCarloNode* bestChild = highestUCB1Child(totalSimCount, &bestMove);
        board.applyMove(bestMove);
        float simulationPoints = bestChild->runIterationOnBoard(board, totalSimCount, maxMoveCount, false);
        points += 1 - simulationPoints;
        return simulationPoints;
    }

    // Is leaf
    // If the node is already end of game, return immediately.
    std::vector<Move> initialMoves = board.findPossibleMoves();
    if (initialMoves.size() == 0)
    {
        nodeIterations++;
        return board.isCheck() ? 0.0f : 1.0f;
    }

    // Node has not been simulated yet. If this is a root node, skip simulation.
    if (nodeIterations++ == 0u && !isRoot)
    {
        unsigned int movesLeft = maxMoveCount;
        Color simulationWinner = Color::NONE;
        bool reachedEnd = false;
        
        while (movesLeft-- > 0u)
        {
            std::vector<Move> nextMoves = board.findPossibleMoves();
            if (nextMoves.size() == 0)
            {
                reachedEnd = true;
                if (board.isCheck())
                {
                    // Checkmate
                    simulationWinner = board.getCurrentPlayer() == Color::BLACK ? Color::WHITE : Color::BLACK;
                    break;
                }
                // Stalemate
                simulationWinner = Color::NONE;
                break;
            }
            if (board.insufficientMaterial())
            {
                reachedEnd = true;
                // Insufficient material
                simulationWinner = Color::NONE;
                break;
            }
            int moveIdx = Random::Range(0, (int)nextMoves.size() - 1);
            board.applyMove(nextMoves[moveIdx]);
        }

        if (!reachedEnd)
        {
            float boardEval = BoardEvaluator::evaluateBoard(board);
            float whiteWinProb = (boardEval / (1 + std::abs(boardEval))) * 0.5f + 0.5f;
            float winProb = nodePlayerColor == Color::WHITE ? whiteWinProb : 1.0f - whiteWinProb;
            points += winProb;
            return winProb;
        }
        if (nodePlayerColor == simulationWinner)
        {
            points += 1.0f;
            return 1.0f;
        }   
        if (simulationWinner == Color::NONE)
        {
            points += 0.5f;
            return 0.5f;
        }
        return 0.0f;
    }

    // Expand the node if it has been simulated and did not have children yet.
    possibleMoves = std::move(initialMoves);
    childNodes.resize(possibleMoves.size());
    board.applyMove(possibleMoves[0]);
    return childNodes[0].runIterationOnBoard(board, totalSimCount, maxMoveCount, false);
}

bool MonteCarloNode::isLeaf()
{
    return !childNodes.size();
}

float MonteCarloNode::UCB1(unsigned int totalVisits)
{
    if (!nodeIterations)
    {
        return FLT_MAX;
    }

    float exploitationFactor = points / nodeIterations;
    float explorationFactor = 2 * float(std::sqrt(std::log(totalVisits) / nodeIterations));
    return exploitationFactor + explorationFactor;
}

MonteCarloNode* MonteCarloNode::highestUCB1Child(unsigned int totalVisits, Move* populateMove)
{
    float bestChildUCB1 = -1.0f;
    MonteCarloNode* bestChild = nullptr;
    for (int i = 0; i < childNodes.size(); i++)
    {
        float thisChildUCB1 = childNodes[i].UCB1(totalVisits);
        if (thisChildUCB1 > bestChildUCB1)
        {
            *populateMove = possibleMoves[i];
            bestChild = &childNodes[i];
            bestChildUCB1 = thisChildUCB1;
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
    float bestWinRate = -1.0f;
    Move bestMove;
    for (int i = 0; i < childNodes.size(); i++)
    {
        if (childNodes[i].nodeIterations == 0u)
            continue;

        float childWinrate = childNodes[i].points / childNodes[i].nodeIterations;
        if (childWinrate > bestWinRate)
        {
            bestWinRate = childWinrate;
            bestMove = possibleMoves[i];
        }
    }
    assert(bestWinRate != -1.0f && "Shouldn't call this function if no iterations have been run.");
    return bestMove;
}

MonteCarloNode MonteCarloNode::getNodeForMove(const Move& move)
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