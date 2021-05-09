#include "TimeManagement.h"

namespace TimeManagement
{
    // Placeholder implementation
    bool timeToMove(float timeLeftMs, float incrementMs, int moveNumber, Duration timeUsed, float confidence)
    {
        if (confidence > 0.9f)
            return true;

        if (timeUsed.count() * 1000 < incrementMs)
            return false;

        // Use no more than 5% of remaining time.
        if (timeUsed.count() * 1000 > timeLeftMs * 0.05f)
            return true;

        return false;
    }
}