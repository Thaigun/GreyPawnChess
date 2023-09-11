#pragma once

#include "../GreyPawnChess.h"

class RandomStrategy : public GreyPawnChess
{
protected:
    void tickComputation() override;
    void applyMoveToStrategy(const Move& move) override;
    Move getBestMove() override;
};