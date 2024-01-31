#ifndef GAME_MATH_H
#define GAME_MATH_H

#include "include/types.h"

#define PI 3.1415926

inline float clamp(float value, float min, float max) 
{
    if (value >= max) 
        return max;
    else if (value <= min)
        return min;
    return value;
}

inline float min(float a, float b) 
{
    return a > b ? b : a;
}

inline u32 max(u32 a, u32 b)
{
    return a > b? a : b;
}

inline V3 cross(V3 a, V3 b)
{
    V3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

V3 norm(V3 a);
float radians(float degrees);

#endif
