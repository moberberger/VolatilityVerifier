#pragma once

#include <intrin.h>

#define TIMER_OVERHEAD_TICKS 24


class Timer
{
public:
    uint64_t accumulatedTicks;
    uint64_t lastStartTime;
    int count;
    int bestSample;
    const char* timerName;

    Timer( const char* _timerName )
    {
        accumulatedTicks = count = 0;
        bestSample = INT_MAX;
        timerName = _timerName;
    }


    inline void Start()
    {
        _mm_lfence();

        unsigned int tmp;
        lastStartTime = __rdtscp( &tmp );
    }

    inline void End()
    {
        _mm_mfence();

        unsigned int tmp;
        auto endTime = __rdtscp( &tmp );
        count++;
        auto delta = endTime - lastStartTime;
        accumulatedTicks += delta;
        bestSample = std::min( bestSample, (int) delta );
    }

    inline const char* ToString()
    {
        double average = (double) accumulatedTicks / count - TIMER_OVERHEAD_TICKS;

        static char str[1024];
        sprintf_s( str, 1024, "%40s: Average= %-20lf Best= %d", timerName, average, bestSample - TIMER_OVERHEAD_TICKS );
        return str;
    }

    inline const char* ForExcel( bool printName = true )
    {
        double average = (double) accumulatedTicks / count - TIMER_OVERHEAD_TICKS;

        static char str[1024];
        if (printName)
            sprintf_s( str, 1024, "\"%s\"\t%lf\t%d", timerName, average, bestSample - TIMER_OVERHEAD_TICKS );
        else
            sprintf_s( str, 1024, "%lf\t%d", average, bestSample - TIMER_OVERHEAD_TICKS );

        return str;
    }

    operator const char* () { return ToString(); }
};