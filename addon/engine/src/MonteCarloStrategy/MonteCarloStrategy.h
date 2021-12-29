#pragma once

#include "MonteCarloNode.h"
#include "../GreyPawnChess.h"

class MonteCarloStrategy : public GreyPawnChess
{
protected:
    void tickComputation() override;
    void applyMoveToStrategy(const Move& move) override;
    Move getBestMove() override;

private:
    MonteCarloNode monteCarloTree;
};