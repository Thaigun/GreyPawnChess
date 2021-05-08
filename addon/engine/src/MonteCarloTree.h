#pragma once

#include <memory>

#include "MonteCarloNode.h"

class MonteCarloTree 
{
public:
    void runIteration();
    Move getBestOption();
    void applyMove(const Move& move);

private:
    MonteCarloNode root;
    unsigned int totalSimulations = 0u;
};