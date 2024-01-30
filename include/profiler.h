//
// Created by stachelbeere1248 on 1/30/24.
//

#ifndef PROFILER_H
#define PROFILER_H
#include <ctime>

struct Profiler {
    timespec start;
    timespec end;
};

#endif //PROFILER_H
