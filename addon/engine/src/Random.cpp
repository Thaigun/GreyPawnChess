#include "Random.h"

#include <assert.h>
#include <chrono>
#include <random>

int Random::Range(int min, int max)
{
    assert(min <= max);
    static std::default_random_engine rng((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(rng);
}

unsigned int Random::Range(unsigned int min, unsigned int max)
{
    assert(min <= max);
    static std::default_random_engine rng((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<unsigned int> distribution(min, max);
    return distribution(rng);
}

float Random::Range(float min, float max)
{
    assert(min <= max);
    static std::default_random_engine rng((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(rng);
}
