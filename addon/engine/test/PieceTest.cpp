#include <gtest/gtest.h>

#include "../src/Piece.h"

// Demonstrate some basic assertions.
TEST(PieceTest, Operators) 
{
	Piece whiteBishop = Piece::WHITE | Piece::BISHOP;
	EXPECT_EQ(Piece::WHITE, whiteBishop & Piece::WHITE);
	EXPECT_TRUE(!!(whiteBishop & Piece::WHITE));
	EXPECT_TRUE(!!(whiteBishop & Piece::BISHOP));
	EXPECT_FALSE(!!(whiteBishop & Piece::BLACK));
	EXPECT_FALSE(!!(whiteBishop & Piece::QUEEN));
}