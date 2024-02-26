#ifndef PROFILER_H
#define PROFILER_H

#include "include/types.h"

enum LogTarget
{
    LogTarget_GameRaycast,
    LogTarget_Backend,
    LogTarget_InterpolatePose,

    LogTarget_Count
};

struct LogEntry
{
    u32 count;
    float total_duration;
};

struct LogEntryInfo
{
    LogTarget target;
    double start;
};

struct FrameLog
{
    LogEntry entries[LogTarget_Count];
};

void start_frame();
void end_frame();

LogEntryInfo start_log(LogTarget target);
void end_log(LogEntryInfo info);

#endif 
