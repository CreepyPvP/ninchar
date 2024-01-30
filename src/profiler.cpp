#include "include/profiler.h"

#ifdef WINDOWS
#include <Windows.h>

double wall_time()
{
    LARGE_INTEGER time;
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return 0;
    }
    if (!QueryPerformanceCounter(&time)) {
        return 0;
    }
    return (double) time.QuadPart / freq.QuadPart;
}

#else

#include <ctime>

double wall_time()
{
    timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_sec + timestamp.tv_nsec * 1.0e-9;
}

#endif

void start_timestamp(Profiler* profiler)
{
    profiler->start = wall_time();
}

double end_timestamp(Profiler* profiler)
{
    double end = wall_time();
    return end - profiler->start;
}
