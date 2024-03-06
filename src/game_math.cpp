#include "include/game_math.h"

#include <math.h>

#include <glm/gtc/matrix_transform.hpp>

V3 V3::operator+(V3 other)
{
    return { x + other.x, y + other.y, z + other.z };
}

V3 V3::operator-(V3 other)
{
    return { x - other.x, y - other.y, z - other.z };
}

V3 V3::operator*(float f)
{
    return { x * f, y * f, z * f };
}

V3 Mat3::operator*(V3 vec)
{
    return { v[0] * vec.x + v[3] * vec.y + v[6] * vec.z,
             v[1] * vec.x + v[4] * vec.y + v[7] * vec.z,
             v[2] * vec.x + v[5] * vec.y + v[8] * vec.z };
}

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

Mat4 mat4(Mat3 mat)
{
    Mat4 res;
    res[0][0] = mat.v[0];
    res[0][1] = mat.v[1];
    res[0][2] = mat.v[2];
    res[0][3] = 0;

    res[1][0] = mat.v[3];
    res[1][1] = mat.v[4];
    res[1][2] = mat.v[5];
    res[1][3] = 0;

    res[2][0] = mat.v[6];
    res[2][1] = mat.v[7];
    res[2][2] = mat.v[8];
    res[2][3] = 0;

    res[3][0] = 0;
    res[3][1] = 0;
    res[3][2] = 0;
    res[3][3] = 1;

    return res;
}

Quat quat(Mat3 mat)
{
    glm::mat3 rot = *((glm::mat3*) &mat);
    return glm::quat_cast(rot);
}

V3 v2int_to_v3float(V2int v2, float z){
    return {v2.x/(float)INT_TILE_SIZE, v2.y/(float)INT_TILE_SIZE, z};
}
V2int v3float_to_v2int(V3 v3){
    return {(int) (v3.x * INT_TILE_SIZE), (int) (v3.y*INT_TILE_SIZE) };
}
