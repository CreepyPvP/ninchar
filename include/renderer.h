#ifndef RENDERER_H
#define RENDERER_H

#include "include/types.h"


struct RenderGroup;


struct TextureHandle
{
    u32 id;
};

struct TextureLoadOp
{
    i32 width;
    i32 height;
    i32 num_channels;
    u8* data;
    TextureHandle* handle;
};

struct Vertex
{
    V3 pos;
    V2 uv;
    V3 norm;
    V3 color;
};

struct RenderSetup
{
    Mat4 proj;
    bool lit;
    bool culling;
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

struct RenderSettings
{
    u32 width;
    u32 height;
};

struct CommandBuffer
{
    RenderSettings settings;

    Vertex* vert_buffer;
    TextureHandle* texture_buffer;
    u32 quad_cap;
    u32 quad_count;

    u8* entry_buffer;
    u32 entry_cap;
    u32 entry_size;

    TextureHandle white;

    RenderGroup* active_group;
};

struct CommandEntryClear
{
    CommandEntryHeader header;
    V3 color;
};

struct CommandEntryDraw
{
    CommandEntryHeader header;
    Mat4 proj;
    u32 quad_offset;
    u32 quad_count;

    RenderSetup setup;
};

struct RenderGroup
{
    CommandBuffer* commands;
    CommandEntryDraw* current_draw;
    RenderSetup setup;
};


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 quad_cap, Vertex* vert_buffer, TextureHandle* texture_buffer,
                             u32 width, u32 height, TextureHandle white);

RenderGroup render_group(CommandBuffer* commands, Mat4 proj, bool lit, bool culling);

void push_clear(CommandBuffer* buffer, V3 color);
void push_cube(RenderGroup* group, V3 pos, V3 radius, TextureHandle texture, V3 color);
void push_line(RenderGroup* group, V3 start, V3 end, V3 color);

TextureLoadOp texture_load_op(TextureHandle* handle, const char* path);
void free_texture_load_op(TextureLoadOp* load_op);


inline bool equal_settings(RenderSettings* a, RenderSettings* b) 
{
    return (a->width == b->width) && (a->height == b->height);
}

#endif
