#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "../src/Board.h"
#include "../src/Move.h"
#include "../src/ScopedProfiler.h"
#include "../src/StringUtil.h"

unsigned int countPossibleMoves(const Board& board, unsigned int depth, bool divide = false)
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
		unsigned int nextBoardMoves = countPossibleMoves(nextBoard, depth - 1);
		foundMoves += nextBoardMoves;
		if (divide)
		{
			std::cout << move.asUCIstr() << ": " << nextBoardMoves << std::endl;
		}
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
	int depth = 5;
	unsigned int foundMoves = countPossibleMoves(board, depth);
	ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	PROFILER_RESET();
}

TEST(BoardTest, LegalMoves2) 
{
	const Board board = Board::buildFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
	unsigned int expectedMoveCounts[5] = {
		1u, 48u, 2039u, 97862u, 4085603u
	};
	int depth = 3;
	unsigned int foundMoves = countPossibleMoves(board, depth);
	ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	PROFILER_RESET();
}

TEST(BoardTest, LegalMoves3) 
{
	const Board board = Board::buildFromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
	unsigned int expectedMoveCounts[7] = {
		1u, 14u, 191u, 2812u, 43238u, 674624u, 11030083u
	};
	int depth = 5;
	unsigned int foundMoves = countPossibleMoves(board, depth);
	ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	PROFILER_RESET();
}

TEST(BoardTest, LegalMoves4) 
{
	const Board board = Board::buildFromFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
	unsigned int expectedMoveCounts[6] = {
		1u, 6u, 264u, 9467u, 422333u, 15833292u
	};
	int depth = 4;
	unsigned int foundMoves = countPossibleMoves(board, depth);
	ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	PROFILER_RESET();
}

TEST(BoardTest, LegalMoves5) 
{
	const Board board = Board::buildFromFEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
	unsigned int expectedMoveCounts[6] = {
		1u, 44u, 1486u, 62379u, 2103487u, 89941194u
	};
	int depth = 4;
	unsigned int foundMoves = countPossibleMoves(board, depth);
	ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	PROFILER_RESET();
}

TEST(BoardTest, LegalMoves6) 
{
	const Board board = Board::buildFromFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
	unsigned int expectedMoveCounts[6] = {
		1u, 46u, 2079u, 89890u, 3894594u, 164075551u
	};
	int depth = 4;
	unsigned int foundMoves = countPossibleMoves(board, depth);
	ASSERT_EQ(foundMoves, expectedMoveCounts[depth]);
	PROFILER_RESET();
}

TEST(BoardTest, InsufficientMaterial)
{
	// Rooks make the material sufficient
	Board board = Board::buildFromFEN("8/2k5/8/8/5R2/8/4K3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("8/2k5/8/8/5R2/8/3RK3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("8/2k5/3r4/8/8/8/3RK3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	
	// A pawn makes the material sufficient
	board = Board::buildFromFEN("8/2k5/5P2/8/8/8/4K3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("8/2k5/8/8/8/8/1p2K3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("8/2k5/8/8/8/1P6/1p2K3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());

	// Test different sufficient bishop stups
	board = Board::buildFromFEN("7B/2k5/8/8/8/8/B3K3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("7b/2k5/8/8/8/8/B3K3/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	
	// Test insufficient bishop stups
	board = Board::buildFromFEN("8/2k5/8/8/5B2/8/4K3/8 w - - 0 1");
	EXPECT_TRUE(board.insufficientMaterial());
	board = Board::buildFromFEN("B6k/8/8/8/8/8/8/b6K w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());

	// Test different knight setups
	board = Board::buildFromFEN("k7/2N5/1K6/8/8/8/8/8 w - - 0 1");
	EXPECT_TRUE(board.insufficientMaterial());
	board = Board::buildFromFEN("kn6/2N5/1K6/8/8/8/8/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("k7/2N5/1K6/8/8/8/8/1N6 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());

	// Knigth + bishop
	board = Board::buildFromFEN("8/8/1K6/8/8/4kB2/8/1N6 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("8/5b2/1K6/8/8/4k3/8/1N6 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
	board = Board::buildFromFEN("8/8/1K2b3/6n1/8/4k3/8/8 w - - 0 1");
	EXPECT_FALSE(board.insufficientMaterial());
}

TEST(BoardTest, HashTest0)
{
	Board board1;
	Board board2;
	ASSERT_EQ(board1.getHash(), board2.getHash());
}

TEST(BoardTest, HashTest1) 
{
	// Board that is brought to the position move by move should have the same hash as a board build from the corresponding FEN string
	Board board;
	board.applyMove(board.constructMove("d2d4"));
	board.applyMove(board.constructMove("d7d5"));
	board.applyMove(board.constructMove("b1c3"));
	board.applyMove(board.constructMove("g8f6"));
	unsigned int hash1 = board.getHash();
	Board board2 = Board::buildFromFEN("rnbqkb1r/ppp1pppp/5n2/3p4/3P4/2N5/PPP1PPPP/R1BQKBNR w KQkq - 2 3");
	unsigned int hash2 = board2.getHash();
	ASSERT_EQ(hash1, hash2);
}

TEST(BoardTest, HashTest2)
{
	// Two transpositions should have the same hash
	Board board;
	board.applyMove(board.constructMove("d2d3"));
	board.applyMove(board.constructMove("d7d6"));
	board.applyMove(board.constructMove("b1c3"));
	board.applyMove(board.constructMove("g8f6"));
	unsigned int hash1 = board.getHash();
	Board board2;
	board2.applyMove(board2.constructMove("b1c3"));
	board2.applyMove(board2.constructMove("g8f6"));
	board2.applyMove(board2.constructMove("d2d3"));
	board2.applyMove(board2.constructMove("d7d6"));
	unsigned int hash2 = board2.getHash();
	ASSERT_EQ(hash1, hash2);
}

TEST(BoardTest, HashTest3)
{
	// After two moves from the initial position, there should be 400 different hashes
	std::unordered_set<unsigned int> hashes;
	Board board;
	std::vector<Move> possibleMoves = board.findPossibleMoves();
	for (const Move& move : possibleMoves)
	{
		Board boardCopy = board;
		boardCopy.applyMove(move);
		std::vector<Move> possibleMoves2 = boardCopy.findPossibleMoves();
		for (const Move& move2 : possibleMoves2)
		{
			Board boardCopy2 = boardCopy;
			boardCopy2.applyMove(move2);
			unsigned int hash = boardCopy2.getHash();
			hashes.insert(hash);
		}
	}
	ASSERT_EQ(hashes.size(), 400);
}

TEST(BoardTest, HashTest4)
{
	// Castling rights should be taken into account
	Board board = Board::buildFromFEN("rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
	unsigned int hashBefore = board.getHash();
	board.applyMove(board.constructMove("e1g1"));
	board.applyMove(board.constructMove("b8c6"));
	board.applyMove(board.constructMove("g1e1"));
	board.applyMove(board.constructMove("c6b8"));
	unsigned int hashAfter = board.getHash();
	ASSERT_NE(hashBefore, hashAfter);
}

TEST(BoardTest, HashTest5)
{
	// Moving back and forth should not change the hash (unless castling rights are changed)
	Board board = Board::buildFromFEN("rnbqkb1r/1p2pppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6");
	unsigned int hashBefore = board.getHash();
	board.applyMove(board.constructMove("c1f4"));
	board.applyMove(board.constructMove("d8d7"));
	board.applyMove(board.constructMove("f4c1"));
	board.applyMove(board.constructMove("d7d8"));
	unsigned int hashAfter = board.getHash();
	ASSERT_EQ(hashBefore, hashAfter);
}

TEST(BoardTest, HashTest6)
{
	// Different player in turn should change the hash
	Board board = Board::buildFromFEN("rnbqkb1r/1p2pppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6");
	unsigned int hash1 = board.getHash();
	Board board2 = Board::buildFromFEN("rnbqkb1r/1p2pppp/p2p1n2/8/3N4/2N1P3/PPP2PPP/R1BQKB1R w KQkq - 0 6");
	board2.applyMove(board2.constructMove("e3e4"));
	unsigned int hash2 = board2.getHash();
	ASSERT_NE(hash1, hash2);
}

TEST(BoardTest, HashTest7)
{
	// En passant square should change the hash
	Board board;
	board.applyMove(board.constructMove("d2d4"));
	board.applyMove(board.constructMove("d7d5"));
	unsigned int enPassantHash = board.getHash();
	Board board2;
	board2.applyMove(board2.constructMove("d2d3"));
	board2.applyMove(board2.constructMove("d7d6"));
	board2.applyMove(board2.constructMove("d3d4"));
	board2.applyMove(board2.constructMove("d6d5"));
	unsigned int noEnPassantHash = board2.getHash();
	ASSERT_NE(enPassantHash, noEnPassantHash);
}

TEST(BoardTest, HashTest8)
{
	// Check that taking a piece leads to correct hash
	Board board = Board::buildFromFEN("r1bqk2r/pp3ppp/2nppn2/2p5/2PP4/2PBPN2/P4PPP/R1BQK2R w KQkq - 0 8");
	board.applyMove(board.constructMove("d4c5"));
	unsigned int hash1 = board.getHash();
	Board board2 = Board::buildFromFEN("r1bqk2r/pp3ppp/2nppn2/2P5/2P5/2PBPN2/P4PPP/R1BQK2R b KQkq - 0 8");
	unsigned int hash2 = board2.getHash();
	ASSERT_EQ(hash1, hash2);
}

TEST(BoardTest, HashTest8_1)
{
	// End up to the same position as in the previous test but with different move
	Board board = Board::buildFromFEN("r1bqk2r/pp3ppp/2nppn2/2P5/2P5/2P1PN2/P1B2PPP/R1BQK2R w KQkq - 0 8");
	board.applyMove(board.constructMove("c2d3"));
	unsigned int hash1 = board.getHash();
	Board board2 = Board::buildFromFEN("r1bqk2r/pp3ppp/2nppn2/2P5/2P5/2PBPN2/P4PPP/R1BQK2R b KQkq - 0 8");
	unsigned int hash2 = board2.getHash();
	ASSERT_EQ(hash1, hash2);
}

TEST(BoardTest, HashTest9)
{
	// Promotion with take
	Board board = Board::buildFromFEN("1r2k2r/2P4p/5q2/p7/6P1/5P2/2R2K2/2R5 w k - 0 1");
	board.applyMove(board.constructMove("c7b8q"));
	unsigned int hash1 = board.getHash();
	Board board2 = Board::buildFromFEN("1Q2k2r/7p/5q2/p7/6P1/5P2/2R2K2/2R5 b k - 0 1");
	unsigned int hash2 = board2.getHash();
	ASSERT_EQ(hash1, hash2);
}

TEST(BoardTest, HashTest10)
{
	// Promotion without take
	Board board = Board::buildFromFEN("4k2r/7p/5q2/8/5QP1/5P2/p1R2K2/2R5 b k - 0 1");
	board.applyMove(board.constructMove("a2a1n"));
	unsigned int hash1 = board.getHash();
	Board board2 = Board::buildFromFEN("4k2r/7p/5q2/8/5QP1/5P2/2R2K2/n1R5 w k - 0 1");
	unsigned int hash2 = board2.getHash();
	ASSERT_EQ(hash1, hash2);
}

TEST(BoardTest, SpecificMoves1)
{
	// Test en passant take
	Board board = Board::buildFromFEN("rnbqkb1r/ppp2ppp/8/8/P1PppPn1/8/1P4PP/RNBK1BNR b kq c3 0 7");
	board.applyMove(board.constructMove("d4c3"));
	Piece takenPiece = board.getSquare("c4");
	ASSERT_EQ(takenPiece, Piece::NONE);
}

TEST(BoardTest, SpecificMoves2)
{
	std::string movesStr = "e2e4 e7e5 d1g4 g8f6 a2a4 f6g4 d2d4 e5d4 e1d1 d7d5 f2f4 d5e4 c2c4 d4c3 c1d2 c3d2 d1c2 d2d1q c2c3 c8e6 b2b4 d1d4";
	auto movesList = StringUtil::split(movesStr);
	Board board;
	for (auto move : movesList)
	{
		board.applyMove(board.constructMove(move));
	}
	const std::vector<Move> moves = board.findPossibleMoves();
	ASSERT_EQ(moves.size(), 1);
}