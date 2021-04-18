#pragma once

#include <cstdint>

enum class Piece : uint16_t
{
    NONE = 0,
    PAWN = 1 << 1,
    KNIGHT = 1 << 2,
    BISHOP = 1 << 3,
    ROOK = 1 << 4,
    QUEEN = 1 << 4,
    KING = 1 << 5,
    WHITE = 1 << 6,
    BLACK = 1 << 7,
    COLOR_MASK = WHITE | BLACK
};

constexpr enum Piece operator|(const Piece a, const Piece b) 
{
    return (enum Piece)(uint16_t(a) | uint16_t(b));
}

constexpr enum Piece operator&(const Piece a, const Piece b) 
{
    return (enum Piece)(uint16_t(a) & uint16_t(b));
}

constexpr enum Piece operator~(const Piece a) 
{
    return (enum Piece)(~uint16_t(a));
}

constexpr enum Piece operator^(const Piece a, const Piece b) 
{
    return (enum Piece)(uint16_t(a) ^ uint16_t(b));
}

constexpr bool operator!(const Piece a)
{   
    return a == static_cast<Piece>(uint16_t(0));
}

constexpr enum Piece& operator|=(const Piece& a, const Piece b)
{
    return (enum Piece&)((uint16_t&)a |= (uint16_t)b);
}

constexpr enum Piece& operator&=(const Piece& a, const Piece b)
{
    return (enum Piece&)((uint16_t&)a &= (uint16_t)b);
}

constexpr enum Piece& operator^=(const Piece& a, const Piece b)
{
    return (enum Piece&)((uint16_t&)a ^= (uint16_t)b);
}
