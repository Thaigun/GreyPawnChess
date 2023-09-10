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

    root.runIteration(freshBoard, 0u);
    Move bestMove = root.highestWinrateMove();
    EXPECT_EQ(root.nodeVisits(), 1u);
    MonteCarloNode nextNode = root.getNodeForMove(bestMove);
    EXPECT_EQ(nextNode.nodeVisits(), 1u);
}

TEST(MonteCarloNodeTest, ManyIterations)
{
    MonteCarloNode root;
    Board board;
    unsigned int iterations = 1000u;
    for (unsigned int i = 0u; i < iterations; i++)
    {
        root.runIteration(board, 50u);
    }
    EXPECT_EQ(root.nodeVisits(), iterations);
}

TEST(MonteCarloNodeTest, ForcedMate)
{
    MonteCarloNode root;
    Board forcedMateInTwo = Board::buildFromFEN("2r4k/6pp/5p2/7K/2R1r3/q4n2/2R5/8 w - - 0 1");
    unsigned int iterations = 1000u;
    for (unsigned int i = 0u; i < iterations; i++)
    {
        root.runIteration(forcedMateInTwo, 50u);
    }
    const Move bestMove = root.highestWinrateMove();
    EXPECT_EQ(bestMove.asUCIstr(),"c4c8");
}