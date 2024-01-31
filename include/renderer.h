#ifndef RENDERER_H
#define RENDERER_H

#include "include/types.h"

struct Vertex
{
    V3 pos;
    V2 uv;
    V3 norm;
    V3 color;
};

enum CommandEntryType
{
    EntryType_Clear = 0,
    EntryType_Draw = 1,
};

struct CommandEntryHeader
{
    u32 type;
};

struct CommandBuffer
{
    Vertex* vert_buffer;
    u32 vert_cap;
    u32 vert_count;

    u8* entry_buffer;
    u32 entry_cap;
    u32 entry_size;
};

struct CommandEntry_Clear
{
    CommandEntryHeader header;
    V3 color;
};

struct CommandEntry_Draw
{
    CommandEntryHeader header;
    Mat4 proj;
    u32 vertex_offset;
    u32 quad_count;
};

struct RenderGroup
{
    CommandBuffer* commands;
    CommandEntry_Draw* current_draw;
    Mat4 proj;
};


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 vert_cap, Vertex* vert_buffer);

RenderGroup render_group(CommandBuffer* commands, Mat4 proj);

void push_clear(CommandBuffer* buffer, V3 color);
void push_cube(RenderGroup* group, V3 pos, V3 radius, V3 color);

#endif
