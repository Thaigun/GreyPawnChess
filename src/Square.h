#pragma once

#include <cstdint>

enum class Square : uint16_t
{
    NONE = 0,
    PAWN = 1 << 1,
    KNIGHT = 1 << 2,
    BISHOP = 1 << 3,
    ROOK = 1 << 4,
    QUEEN = 1 << 4,
    KING = 1 << 5,
    WHITE = 1 << 6,
    BLACK = 1 << 7
};

constexpr enum Square operator|(const Square a, const Square b) 
{
    return (enum Square)(uint16_t(a) | uint16_t(b));
}

constexpr enum Square operator&(const Square a, const Square b) 
{
    return (enum Square)(uint16_t(a) & uint16_t(b));
}

constexpr enum Square operator~(const Square a) 
{
    return (enum Square)(~uint16_t(a));
}

constexpr enum Square operator^(const Square a, const Square b) 
{
    return (enum Square)(uint16_t(a) ^ uint16_t(b));
}

constexpr bool operator!(const Square a)
{   
    return a == static_cast<Square>(uint16_t(0));
}

constexpr enum Square& operator|=(const Square& a, const Square b)
{
    return (enum Square&)((uint16_t&)a |= (uint16_t)b);
}

constexpr enum Square& operator&=(const Square& a, const Square b)
{
    return (enum Square&)((uint16_t&)a &= (uint16_t)b);
}

constexpr enum Square& operator^=(const Square& a, const Square b)
{
    return (enum Square&)((uint16_t&)a ^= (uint16_t)b);
}
