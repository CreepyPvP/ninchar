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

    u32* index_buffer;
    u32 index_cap;
    u32 index_count;

    u8* entry_buffer;
    u32 entry_cap;
    u32 entry_size;

    Mat4 proj;
};

struct CommandEntry_Clear
{
    CommandEntryHeader header;
    V3 color;
};


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 vert_cap, Vertex* vert_buffer, 
                             u32 index_cap, u32* index_buffer,
                             Mat4 proj);

void push_clear(CommandBuffer* buffer, V3 color);
void push_quad(CommandBuffer* buffer, V3 color);

#endif
