#include "include/profiler.h"
#include <stdio.h>


// NOTE: wall_time() returns current wall time in seconds

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

double frame_start;
LogEntry entries[LogTarget_Count];

void start_frame()
{
    memset(entries, 0, sizeof(LogEntry) * LogTarget_Count);
    frame_start = wall_time();
}

void end_frame()
{
    double duration = wall_time() - frame_start;
    printf("------------------------\n");
    printf("Frame took %f ms\n", duration * 1000);
    printf("Backend took %f ms\n", entries[LogTarget_Backend].total_duration * 1000);
    printf("Raycast took %f ms, was called: %u\n", entries[LogTarget_GameRaycast].total_duration * 1000, 
           entries[LogTarget_GameRaycast].count);
    printf("Interpolate pose took %f ms\n", entries[LogTarget_InterpolatePose].total_duration * 1000);
}

LogEntryInfo start_log(LogTarget target)
{
    LogEntryInfo info = {};
    info.target = target;
    info.start = wall_time();
    return info;
}

void end_log(LogEntryInfo info)
{
    double duration = wall_time() - info.start;
    entries[info.target].count++;
    entries[info.target].total_duration += duration;
}
