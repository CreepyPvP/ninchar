#include "include/profiler.h"

void Start(Profiler* profiler) {
    clock_gettime(CLOCK_MONOTONIC, &profiler->start);
}
double End(Profiler* profiler) {
    clock_gettime(CLOCK_MONOTONIC, &profiler->end);

    return (double) profiler->end.tv_sec + profiler->end.tv_nsec * 1.0e-9 -
        ((double) profiler->start.tv_sec + profiler->start.tv_nsec * 1.0e-9);
}