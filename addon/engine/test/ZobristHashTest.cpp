#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "../src/Board.h"
#include "../src/Move.h"
#include "../src/StringUtil.h"
#include "../src/ZobristHash.h"


TEST(ZobristHashTest, InitialHashes)
{
	ZobristHash hash1;
	ZobristHash hash2;
	ASSERT_EQ(hash1.getHash(), hash2.getHash());
}

TEST(ZobristHashTest, TogglePiece)
{
    // Toggling should change the hash and toggling back should reset
    ZobristHash hash;
    unsigned int hashVal0 = hash.getHash();
    hash.togglePiece((char)12u, Piece::WHITE | Piece::PAWN);
    unsigned int hashVal1 = hash.getHash();
    hash.togglePiece((char)3u, Piece::WHITE | Piece::BISHOP);
    unsigned int hashVal2 = hash.getHash();
    hash.togglePiece((char)12u, Piece::WHITE | Piece::PAWN);
    unsigned int hashVal3 = hash.getHash();
    hash.togglePiece((char)3u, Piece::WHITE | Piece::BISHOP);
    unsigned int hashVal4 = hash.getHash();
    ASSERT_NE(hashVal0, hashVal1);
    ASSERT_NE(hashVal1, hashVal2);
    ASSERT_NE(hashVal2, hashVal3);
    ASSERT_NE(hashVal0, hashVal3);
    ASSERT_EQ(hashVal0, hashVal4);
}

TEST(ZobristHash, TogglePlayerInTurn)
{
    // Toggling should change the hash and toggling back should reset
    ZobristHash hash;
    unsigned int hashVal0 = hash.getHash();
    hash.togglePlayerInTurn();
    unsigned int hashVal1 = hash.getHash();
    hash.togglePlayerInTurn();
    unsigned int hashVal2 = hash.getHash();
    ASSERT_NE(hashVal0, hashVal1);
    ASSERT_EQ(hashVal0, hashVal2);
}