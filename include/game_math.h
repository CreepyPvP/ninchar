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

inline float max(float a, float b) 
{
    return a > b ? a : b;
}

inline u32 max(u32 a, u32 b)
{
    return a > b? a : b;
}

inline u32 int_max(int a, int b)
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

inline float dot(V3 a, V3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

V3 norm(V3 a);
V2 norm(V2 a);
float radians(float degrees);

// Checks if float in range between VERY_SMALL_VALUE to -VERY_SMALL_VALUE
bool is_zero(float x);

float halton(u32 i, u32 b);

#define INT_TILE_SIZE 1024

V3 v2int_to_v3float(V2int v2, float z);
V2int v3float_to_v2int(V3 v3);

#endif
