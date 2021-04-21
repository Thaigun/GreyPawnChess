#include <gtest/gtest.h>

#include "../src/Move.h"

// Demonstrate some basic assertions.
TEST(MoveTest, CastlingMove) 
{	
	Move castlingMove(4, 6, 7, 5);
	EXPECT_TRUE(castlingMove.isCastling());
	EXPECT_EQ(castlingMove.asUCIstr(), "e1g1");
}

TEST(MoveTest, PromotionMove)
{
	Move promotionKnight(52, 60, Piece::KNIGHT);
	Move promotionQueen(10, 2, Piece::QUEEN);
	EXPECT_TRUE(promotionKnight.isPromotion());
	EXPECT_TRUE(promotionQueen.isPromotion());
	EXPECT_EQ(promotionKnight.from[0], 52);
	EXPECT_EQ(promotionKnight.to[0], 60);
	EXPECT_EQ(promotionKnight.asUCIstr(), "e7e8n");
	EXPECT_EQ(promotionQueen.asUCIstr(), "c2c1q");
}

TEST(MoveTest, NormalMove)
{
	Move normalMove(11, 19);
	EXPECT_FALSE(normalMove.isCastling());
	EXPECT_FALSE(normalMove.isPromotion());
	EXPECT_EQ(normalMove.asUCIstr(), "d2d3");
}