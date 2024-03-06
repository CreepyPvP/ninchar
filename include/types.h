#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <assert.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#define DEBUG
#define WINDOWS

#ifdef WINDOWS
#ifndef DEBUG
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef glm::mat4 Mat4;
typedef glm::quat Quat;


struct Arena;


struct V2int{
    int x;
    int y;
};


struct V2
{
    float x;
    float y;
};

union V3
{
    struct
    {
        float x;
        float y;
        float z;
    };

    float v[3];

    V3 operator+(V3 other);
    V3 operator-(V3 other);
    V3 operator*(float f);
};

union V4
{
    struct
    {
        float x;
        float y;
        float z;
        float w;
    };

    struct
    {
        V3 xyz;
    };

    struct
    {
        V3 rgb;
    };
};

// NOTE: Column major
struct Mat3
{
    float v[9];

    V3 operator*(V3 vec);
};

inline Mat3 mat3(float f)
{
    return { f, 0, 0, 0, f, 0, 0, 0, f };
}

inline V2 v2(float x, float y)
{
    return { x, y };
}

inline V2 v2(float x)
{
    return { x, x };
}

inline V3 v3(float x, float y, float z)
{
    return { x, y, z };
}

inline V3 lerp(V3 a, V3 b, float t)
{
    return v3((1 - t) * a.x + t * b.x, (1 - t) * a.y + t * b.y, (1 - t) * a.z + t * b.z);
}

inline V3 v3(float x)
{
    return { x, x, x };
}

inline V4 v4(float x) 
{
    return { x, x, x, x };
}

Mat4 mat4(V3 pos, V3 scale);
Mat4 mat4(Mat3 mat);

Quat quat(Mat3 mat);

struct Str
{
    char* ptr;
    u16 len;
    u16 cap;
};

Str str_with_cap(u16 cap, Arena* arena);
Str from_c_str(const char* c_str, Arena* arena);
Str str_cpy(Str* str, Arena* arena);
bool str_equals(Str a, Str b);
void append_line(Str* str, const char* line);
void append_null(Str* str);

#include "include/arena.h"

#endif
