#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#define USE_PROFILER 0
#if USE_PROFILER
    #define PROFILE(name) ScopedProfiler profiler(name)
    #define PROFILER_PRINT() MainProfiler::getInstance().print()
    #define PROFILER_RESET() MainProfiler::getInstance().reset()
#else
    #define PROFILE(name)
    #define PROFILER_PRINT()
    #define PROFILER_RESET()
#endif

class MainProfiler
{
public:
    void addEntry(const std::string& name, const std::chrono::high_resolution_clock::duration& dur)
    {
        callCounts[name]++;
        blockDurations[name] += dur;
    }
    void print()
    {
        std::cout << "Profiler results:" << std::endl;
        auto totalTime_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - profilerStartTime);
        std::cout << "Total time: " << 0.001f * totalTime_us.count() << " ms" << std::endl;
        for (auto& callCount : callCounts)
        {
            auto totalDuration_us = std::chrono::duration_cast<std::chrono::microseconds>(blockDurations[callCount.first]).count();
            auto avgDuration_us = totalDuration_us / callCount.second;
            std::cout << callCount.first << ": " << callCount.second << " calls in (ms) " << 0.001f * totalDuration_us << ", avg (us): " << avgDuration_us << std::endl;
        }
    }
    void reset()
    {
        callCounts.clear();
        blockDurations.clear();
        profilerStartTime = std::chrono::high_resolution_clock::now();
    }
private:
    std::unordered_map<std::string, unsigned int> callCounts;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::duration> blockDurations;
    std::chrono::high_resolution_clock::time_point profilerStartTime;

// Singleton implementation
public:
    static MainProfiler& getInstance()
    {
        static MainProfiler instance;
        return instance;
    }
    MainProfiler(const MainProfiler&) = delete;
    MainProfiler& operator=(MainProfiler const&) = delete;
private:
    MainProfiler() : profilerStartTime(std::chrono::high_resolution_clock::now()) {}
};

class ScopedProfiler
{
public:
    ScopedProfiler(std::string name)
        : name(name)
        , start(std::chrono::high_resolution_clock::now())
    {
    }
    ~ScopedProfiler()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        // Call a static container for all the profilers
        MainProfiler::getInstance().addEntry(name, duration);
    }

private:
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};