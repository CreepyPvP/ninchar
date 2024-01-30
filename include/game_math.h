#ifndef GAME_MATH_H
#define GAME_MATH_H

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

#endif
