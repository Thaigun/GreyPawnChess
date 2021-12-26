#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#define USE_PROFILER 1
#if USE_PROFILER
    #define PROFILE(name) ScopedProfiler profiler(name);
#else
    #define PROFILE(name)
#endif


class Profiler
{
public:
    static void addEntry(/*std::string& name, */std::chrono::duration<float>& dur) 
    {
        callCounts++;
        durations += dur;
    }
    static void print()
    {
        //for (auto& kv : callCounts)
        //{
            //std::cout << kv.first << ": " << kv.second << " calls, " << durations[kv.first].count() << "us" << std::endl;
        //}
        std::cout << "Total: " << callCounts << " calls, " << durations.count() << "us" << std::endl;
    }
    static void reset()
    {
        //callCounts.clear();
        //durations.clear();
    }
    
    static int callCounts;
    static std::chrono::duration<float> durations;
};

class ScopedProfiler
{
public:
    ScopedProfiler(std::string name)
        : name(name)
        , start(std::chrono::system_clock::now())
    {
    }
    ~ScopedProfiler()
    {
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<float> duration = end - start;
        // Call a static container for all the profilers
        Profiler::addEntry(/*name, */duration);
        std::cout << name << ": " << duration.count() * 1000 << "ms" << std::endl;
    }

private:
    std::string name;
    std::chrono::time_point<std::chrono::system_clock> start;
};