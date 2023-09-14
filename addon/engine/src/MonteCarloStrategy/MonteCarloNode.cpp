#include "MonteCarloNode.h"

#include <assert.h>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>

#include "../Board.h"
#include "../BoardEvaluator.h"
#include "../Random.h"

void MonteCarloNode::runIteration(const Board &board, unsigned int maxMoveCount)
{
    Board iterationBoard = board;
    runIterationOnBoard(iterationBoard, maxMoveCount, true);
}

float MonteCarloNode::runIterationOnBoard(Board& board, unsigned int maxMoveCount, bool isRoot)
{
    nodeIterations++;
    float playoutResult;
    if (conclusiveResult)
    {
        playoutResult = points / nodeIterations;
    }
    else if (nodeIterations == 1u && !isRoot)
    {
        playoutResult = randomPlayout(board, maxMoveCount);
    }
    else
    {
        if (childNodes.size() == 0u)
            expand(board);
        playoutResult = runOnBestChild(board, maxMoveCount);
    }
    points += playoutResult;
    return playoutResult;
}

float MonteCarloNode::runOnBestChild(Board& board, unsigned int maxMoveCount)
{
    Move bestMove;
    MonteCarloNode* bestChild = highestUCB1Child(&bestMove);
    board.applyMove(bestMove);
    float childResult = bestChild->runIterationOnBoard(board, maxMoveCount);
    return 1.0f - childResult;
}

float MonteCarloNode::UCB1(unsigned int totalVisits, bool inversePoints)
{
    if (!nodeIterations)
    {
        return FLT_MAX;
    }

    float exploitationFactor = points / nodeIterations;
    if (inversePoints)
    {
        exploitationFactor = 1.0f - exploitationFactor;
    }
    float explorationFactor = 2 * float(std::sqrt(std::log(totalVisits) / nodeIterations));
    return exploitationFactor + explorationFactor;
}

MonteCarloNode *MonteCarloNode::highestUCB1Child(Move *populateMove)
{
    if (childNodes.size() == 0u)
        return nullptr;

    float bestChildUCB1 = -1.0f;
    std::vector<int> bestChildIndices;
    for (int i = 0; i < childNodes.size(); i++)
    {
        float thisChildUCB1 = childNodes[i].UCB1(nodeIterations, true);
        if (thisChildUCB1 > bestChildUCB1)
        {
            bestChildIndices.clear();
            bestChildIndices.push_back(i);   
            bestChildUCB1 = thisChildUCB1;
        }
        else if (thisChildUCB1 == bestChildUCB1)
        {
            bestChildIndices.push_back(i);
        }
    }
    const int bestChildIdx = bestChildIndices[Random::Range(0, (int)bestChildIndices.size() - 1)];
    *populateMove = possibleMoves[bestChildIdx];
    return &childNodes[bestChildIdx];
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

        float childWinrate = 1.0f - (childNodes[i].points / childNodes[i].nodeIterations);
        if (childWinrate > bestWinRate)
        {
            bestWinRate = childWinrate;
            bestMove = possibleMoves[i];
        }
    }
    assert(bestWinRate != -1.0f && "Shouldn't call this function if no iterations have been run.");
    return bestMove;
}

MonteCarloNode MonteCarloNode::getNodeForMove(const Move &move)
{
    // If this node has not been expanded yet, just return a new node.
    if (childNodes.size() == 0u)
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

void MonteCarloNode::expand(const Board& board)
{
    possibleMoves = board.findPossibleMoves();
    childNodes.resize(possibleMoves.size());
}

float MonteCarloNode::randomPlayout(Board &board, unsigned int maxMoveCount)
{
    std::vector<Move> nextMoves = board.findPossibleMoves();
    if (nextMoves.size() == 0)
    {
        conclusiveResult = true;
        return board.isCheck() ? 0.0f : 0.5f;
    }

    unsigned int movesLeft = maxMoveCount;
    Color nodeColor = board.getCurrentPlayer();
    while (movesLeft-- > 0u)
    {
        if (nextMoves.size() == 0)
        {
            if (board.isCheck())
            {
                return board.getCurrentPlayer() == nodeColor ? 0.0f : 1.0f;
            }
            return 0.5f;
        }
        if (board.insufficientMaterial() || board.noProgress() || board.threefoldRepetition())
        {
            return 0.5f;
        }
        int moveIdx = Random::Range(0, (int)nextMoves.size() - 1);
        board.applyMove(nextMoves[moveIdx]);
        nextMoves = board.findPossibleMoves();
    }
    
    float boardEval = BoardEvaluator::evaluateBoard(board);
    float whiteWinProb = (boardEval / (1 + std::abs(boardEval))) * 0.2f + 0.5f;
    float winProb = nodeColor == Color::WHITE ? whiteWinProb : 1.0f - whiteWinProb;
    return winProb;
}

void MonteCarloNode::printStats() const
{
    std::cout << "Node iterations: " << nodeIterations << std::endl;
    std::cout << "Possible moves: " << possibleMoves.size() << std::endl;
    std::cout << "Child nodes: " << childNodes.size() << std::endl;
    std::cout << "Points: " << points << std::endl;
}
