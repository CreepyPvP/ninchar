#include "include/game_math.h"

#include <math.h>

V3 norm(V3 a)
{
    float len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    if (len < 0.0001) {
        return v3(0);
    }
    return v3(a.x / len, a.y / len, a.z / len);
}

V2 norm(V2 a)
{
    float len = sqrt(a.x * a.x + a.y * a.y);
    if (len < 0.0001) {
        return v2(0);
    }
    return v2(a.x / len, a.y / len);
}

bool is_zero(float x)
{
    return (x < 0.00001 && x > -0.00001);
}

float radians(float degrees)
{
    return degrees / 360 * 2 * PI;
}

float halton(u32 i, u32 b)
{
    float f = 1;
    float r = 0;
    while (i > 0) {
        f = f / b;
        r += f * (i % b);
        i = floor(i / b);
    }

    return r;
}
