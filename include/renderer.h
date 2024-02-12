#ifndef RENDERER_H
#define RENDERER_H

#include "include/types.h"
#include "include/arena.h"


struct RenderGroup;


struct TextureHandle
{
    u64 id;
};

struct ModelHandle
{
    u32 id;
    u32 index_count;
};

struct TextureLoadOp
{
    i32 width;
    i32 height;
    i32 num_channels;
    u8* data;
    TextureHandle* handle;
};

struct ModelLoadOp
{
    u8* vert_buffer;
    u32 vert_count;

    u32* index_buffer;
    u32 index_count;

    u32 vert_stride;

    ModelHandle* handle;
};

struct Vertex
{
    V3 pos;
    V2 uv;
    V3 norm;
    V3 color;

    u64 texture;
};

struct RenderSetup
{
    Mat4 proj;
    bool lit;
    bool culling;
    bool shadow_caster;
};

enum CommandEntryType
{
    EntryType_Clear,
    EntryType_DrawQuads,
    EntryType_DrawModel,
    EntryType_PushLight,
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
    u32 vert_count;
    u32 vert_cap;

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

struct CommandEntryDrawQuads
{
    CommandEntryHeader header;
    u32 vert_offset;
    u32 quad_count;
    RenderSetup setup;
};

struct CommandEntryDrawModel
{
    CommandEntryHeader header;
    Mat4 trans;
    ModelHandle model;
    RenderSetup setup;
};

struct CommandEntryPushLight
{
    CommandEntryHeader header;
    V3 pos;
    V3 dir;

    Mat4 light_space;

    float fov;
};

struct RenderGroup
{
    CommandBuffer* commands;
    CommandEntryDrawQuads* current_draw;
    RenderSetup setup;
};


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 vert_cap, Vertex* vert_buffer, 
                             u32 width, u32 height, TextureHandle white);

RenderGroup render_group(CommandBuffer* commands, Mat4 proj, bool lit, bool culling, 
                         bool shadow_caster);

void push_clear(CommandBuffer* buffer, V3 color);
void push_cube(RenderGroup* group, V3 pos, V3 radius, TextureHandle texture, V3 color);
void push_model(RenderGroup* group, ModelHandle handle, V3 pos, V3 scale);
void push_line(RenderGroup* group, V3 start, V3 end, V3 color);

void push_spotlight(CommandBuffer* buffer, V3 pos, V3 dir, float fov);

TextureLoadOp texture_load_op(TextureHandle* handle, const char* path);
void free_texture_load_op(TextureLoadOp* load_op);

ModelLoadOp model_load_op(ModelHandle* handle, const char* path, Arena* arena);



inline bool equal_settings(RenderSettings* a, RenderSettings* b) 
{
    return (a->width == b->width) && (a->height == b->height);
}

#endif
