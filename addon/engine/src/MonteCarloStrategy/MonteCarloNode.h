#pragma once

#include <vector>

#include "../GameState.h"
#include "../Move.h"

class Board;

class MonteCarloNode
{
public:
    // Simulates the given board until the game ends or max number of moves are reached.
    void runIteration(const Board& board, unsigned int maxMoveCount = 15u);
    void prepareRootNode(const Board& board);
    float UCB1(unsigned int totalVisits, bool inversePoints = false);
    MonteCarloNode* highestUCB1Child(Move* populateMove);
    unsigned int nodeVisits() const;
    Move highestWinrateMove() const;
    MonteCarloNode getNodeForMove(const Move& move);
    void printStats() const;

private:
    // Actual implementation is this method. The public version takes a const reference to the board for safety reasons.
    float runIterationOnBoard(Board& board, unsigned int maxMoveCount, bool isRoot = false);
    float runOnBestChild(Board& board, unsigned int maxMoveCount);
    void expand(const Board& board);
    float randomPlayout(Board& board, unsigned int maxMoveCount);

    std::vector<Move> possibleMoves;
    std::vector<MonteCarloNode> childNodes;
    float points = 0.0f;
    unsigned int nodeIterations = 0u;
    bool conclusiveResult = false;
};