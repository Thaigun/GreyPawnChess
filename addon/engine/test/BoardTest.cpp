#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "../src/Board.h"
#include "../src/Move.h"

unsigned int countPossibleMoves(const Board& board, unsigned int depth)
{
	if (depth == 0u)
	{
		return 1;
	}

	unsigned int foundMoves = 0u;
	std::vector<Move> possibleMoves = board.findPossibleMoves();
	for (const Move& move : possibleMoves)
	{
		Board nextBoard = board;
		nextBoard.applyMove(move);
		foundMoves += countPossibleMoves(nextBoard, depth - 1);
	}
	return foundMoves;
}

// https://www.chessprogramming.org/Perft_Results
TEST(BoardTest, LegalMoves1) 
{
	const Board board = Board::buildFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	unsigned int expectedMoveCounts[6] = {
		1u, 20u, 400u, 8902u, 197281u, 4865609u
	};
	for (int depth = 1; depth < 4; depth++)
	{
		unsigned int foundMoves = countPossibleMoves(board, depth);
		ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	}
}

TEST(BoardTest, LegalMoves2) 
{
	const Board board = Board::buildFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
	unsigned int expectedMoveCounts[5] = {
		1u, 48u, 2039u, 97862u, 4085603u
	};
	for (int depth = 1; depth < 2; depth++)
	{
		unsigned int foundMoves = countPossibleMoves(board, depth);
		ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	}
}

TEST(BoardTest, LegalMoves3) 
{
	const Board board = Board::buildFromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
	unsigned int expectedMoveCounts[7] = {
		1u, 14u, 191u, 2812u, 43238u, 674624u, 11030083u
	};
	for (int depth = 1; depth < 2; depth++)
	{
		unsigned int foundMoves = countPossibleMoves(board, depth);
		ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	}
}

TEST(BoardTest, LegalMoves4) 
{
	const Board board = Board::buildFromFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
	unsigned int expectedMoveCounts[6] = {
		1u, 6u, 264u, 9467u, 422333u, 15833292u
	};
	for (int depth = 1; depth < 2; depth++)
	{
		unsigned int foundMoves = countPossibleMoves(board, depth);
		ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	}
}