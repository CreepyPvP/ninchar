#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <assert.h>

#include <glm/glm.hpp>

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


struct V2
{
    float x;
    float y;
};

struct V3
{
    float x;
    float y;
    float z;
};

inline V3 v3(float x, float y, float z)
{
    return { x, y, z };
}

inline V3 v3(float x)
{
    return { x, x, x };
}

#endif