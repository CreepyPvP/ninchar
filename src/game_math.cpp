#include "include/game_math.h"

#include <math.h>

#include <glm/gtc/matrix_transform.hpp>

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

Mat4 mat4(V3 pos, V3 scale)
{
    Mat4 res = glm::mat4(1);
    res = glm::translate(res, glm::vec3(pos.x, pos.y, pos.z));
    res = glm::scale(res, glm::vec3(scale.x, scale.y, scale.z));
    return res;
}


V2 v2int_to_v2float(V2int v2){
    return {v2.x/1000.0f, v2.y/1000.0f};
}
V2int v2float_to_v2int(V2 v2){
    return {(int) (v2.x * 1000.0f), (int) (v2.y*1000.0f) };
}
