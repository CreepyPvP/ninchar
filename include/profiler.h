#ifndef PROFILER_H
#define PROFILER_H

#include "include/types.h"

struct Profiler {
    double start;
};

void start_timestamp(Profiler* profiler);

// NOTE: elapsed time in seconds
double end_timestamp(Profiler* profiler);

#endif 
