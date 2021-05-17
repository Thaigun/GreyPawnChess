#include <algorithm>

#include "TimeManagement.h"

namespace TimeManagement
{
    // Placeholder implementation
    bool timeToMove(float timeLeftMs, float incrementMs, int moveNumber, Duration timeUsed, float confidence)
    {
        int expectedMovesLeft = 10 + std::max(0, 60 - moveNumber);
        if (confidence > 0.9f)
            return true;

        if (timeUsed.count() * 1000 < incrementMs)
            return false;

        // Use no more than 5% of remaining time.
        if (timeUsed.count() * 1000 > timeLeftMs / expectedMovesLeft)
            return true;

        return false;
    }
}