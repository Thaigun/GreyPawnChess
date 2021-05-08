#pragma once

#include <vector>

#include "Board.h"
#include "GameState.h"
#include "Move.h"

class MonteCarloNode
{
public:
    // Simulates the given board until the game ends or max number of moves are reached.
    Color runIteration(Board& board, unsigned int totalSimCount, unsigned int maxMoveCount = 1000u);
    bool isLeaf();
    float UCB1(unsigned int totalVisits, Color player);
    MonteCarloNode* highestUCB1Child(unsigned int totalVisits, Color player, const Move* populateMove);
    unsigned int nodeVisits() const;
    Move highestWinrateMove() const;

private:
    std::vector<Move> possibleMoves;
    std::vector<MonteCarloNode> childNodes;
    float points;
    unsigned int nodeIterations;
};