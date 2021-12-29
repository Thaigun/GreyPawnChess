#include <limits>

#include <gtest/gtest.h>

#include "../src/Board.h"
#include "../src/MonteCarloNode.h"

// Demonstrate some basic assertions.
TEST(MonteCarloNodeTest, FreshNode) 
{	
	MonteCarloNode root;
    EXPECT_TRUE(root.isLeaf());
    EXPECT_EQ(root.nodeVisits(), 0u);
    Move bestMove;
    EXPECT_EQ(root.highestUCB1Child(0u, &bestMove), nullptr);
    EXPECT_EQ(root.UCB1(0u), FLT_MAX);
}

TEST(MonteCarloNode, FirstIteration)
{
    MonteCarloNode root;
    Board freshBoard;
    std::vector<Move> possibleMoves = freshBoard.findPossibleMoves();

    root.runIteration(freshBoard, 0);
    EXPECT_FALSE(root.isLeaf());
    Move bestMove = root.highestWinrateMove();
    bool bestMoveExists = false;
    for (const Move& m : possibleMoves)
    {
        if (bestMove == m)
        {
            bestMoveExists = true;
            break;
        }
    }
    EXPECT_TRUE(bestMoveExists);
    EXPECT_EQ(root.nodeVisits(), 1u);
    MonteCarloNode nextNode = root.getNodeForMove(bestMove);
    EXPECT_TRUE(nextNode.isLeaf());
    EXPECT_EQ(nextNode.nodeVisits(), 1u);
}

TEST(MonteCarloNode, ManyIterations)
{
    MonteCarloNode root;
    Board board;
    unsigned int iterations = 1000;
    for (unsigned int i = 0u; i < iterations; i++)
    {
        root.runIteration(board, root.nodeVisits());
    }
    EXPECT_EQ(root.nodeVisits(), iterations);
}
