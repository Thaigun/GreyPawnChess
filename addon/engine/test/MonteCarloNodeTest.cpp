#include <limits>

#include <gtest/gtest.h>

#include "../src/Board.h"
#include "../src/MonteCarloStrategy/MonteCarloNode.h"

TEST(MonteCarloNodeTest, FreshNode) 
{	
	MonteCarloNode root;
    EXPECT_EQ(root.nodeVisits(), 0u);
    Move bestMove;
    EXPECT_EQ(root.highestUCB1Child(&bestMove), nullptr);
    EXPECT_EQ(root.UCB1(0u), FLT_MAX);
}

TEST(MonteCarloNodeTest, FirstIteration)
{
    MonteCarloNode root;
    Board freshBoard;

    root.runIteration(freshBoard, 0);
    Move bestMove = root.highestWinrateMove();
    EXPECT_EQ(root.nodeVisits(), 1u);
    MonteCarloNode nextNode = root.getNodeForMove(bestMove);
    EXPECT_EQ(nextNode.nodeVisits(), 1u);
}

TEST(MonteCarloNodeTest, ManyIterations)
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
