#pragma once

#include <vector>

#include "../GameState.h"
#include "../Move.h"

class Board;

class MonteCarloNode
{
public:
    // Simulates the given board until the game ends or max number of moves are reached.
    float runIteration(const Board& board, unsigned int totalSimCount, unsigned int maxMoveCount = 15u);
    bool isLeaf();
    float UCB1(unsigned int totalVisits);
    MonteCarloNode* highestUCB1Child(unsigned int totalVisits, Move* populateMove);
    unsigned int nodeVisits() const;
    Move highestWinrateMove() const;
    MonteCarloNode getNodeForMove(const Move& move);

private:
    // Actual implementation is this method. The public version takes a const reference to the board for safety reasons.
    float runIterationOnBoard(Board& board, unsigned int totalSimCount, unsigned int maxMoveCount, bool isRoot);

    std::vector<Move> possibleMoves;
    std::vector<MonteCarloNode> childNodes;
    float points = 0.0f;
    unsigned int nodeIterations = 0u;
};