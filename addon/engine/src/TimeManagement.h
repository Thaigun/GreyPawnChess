#pragma once

#include <chrono>

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;
typedef std::chrono::duration<float> Duration;

namespace TimeManagement
{
    bool timeToMove(float timeLeftMs, float incrementMs, int moveNumber, Duration timeUsed, float confidence);
}