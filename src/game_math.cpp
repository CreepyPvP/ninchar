#include "include/game_math.h"

#include <math.h>

V3 norm(V3 a)
{
    float len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    return v3(a.x / len, a.y / len, a.z / len);
}

float radians(float degrees)
{
    return degrees / 360 * 2 * PI;
}
