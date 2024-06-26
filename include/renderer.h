#ifndef RENDERER_H
#define RENDERER_H

#include "include/types.h"
#include "include/arena.h"

#define MAX_BONE_INFLUENCE 3

#define MODEL_FLAGS_UV (1 << 0)
#define MODEL_FLAGS_RIGGED (1 << 1)

#define RENDER_DEPTH_TEST (1 << 0)
#define RENDER_LIT (1 << 1)
#define RENDER_CULLING (1 << 2)
#define RENDER_SHADOW_CASTER (1 << 3)

struct RenderGroup;

struct Vertex
{
    V3 pos;
    V2 uv;
    V3 norm;
    V3 color;

    u64 texture;
};

struct MeshVertex
{
    V3 pos;
    V2 uv;
    V3 norm;
    V3 color;

    i32 bone_ids[MAX_BONE_INFLUENCE];
    float bone_weights[MAX_BONE_INFLUENCE];   
};

struct BoneInfo
{
    Str name;
    Mat4 offset;
};

struct Skeleton
{
    u32 bone_count;
    u32 bone_cap;
    BoneInfo* bone;
};

struct TextureHandle
{
    u64 id;
};

struct ModelHandle
{
    u32 id;
};

struct RiggedModelHandle
{
    ModelHandle model;
    Skeleton skeleton;
};

struct TextureLoadOp
{
    TextureHandle* handle;
    i32 width;
    i32 height;
    i32 num_channels;
    u8* data;
};

struct MeshInfo
{
    u32 vertex_count;
    MeshVertex* vertex_buffer;
    u32 index_count;
    u32* index_buffer;

    u32 flags;
};

struct ModelLoadOp
{
    ModelHandle* handle;

    u32 mesh_cap;
    u32 mesh_count;
    MeshInfo* meshes;
};

struct RenderSetup
{
    u32 flags;
};

enum CommandEntryType
{
    EntryType_Clear,
    EntryType_DrawQuads,
    EntryType_DrawModel,
    EntryType_DrawRiggedModel,
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

    V3 camera_pos;
    V3 camera_up;
    V3 camera_right;
    Mat4 proj;

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

struct CommandEntryDrawRiggedModel
{
    CommandEntryHeader header;
    Mat4 trans;
    ModelHandle model;
    RenderSetup setup;

    u32 bone_count;
    Mat4* bone_trans;
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

CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, u32 vert_cap, Vertex* vert_buffer, 
                             u32 width, u32 height, TextureHandle white,
                             Mat4 proj, V3 camera_pos, V3 camera_right, V3 camera_up);

RenderGroup render_group(CommandBuffer* commands, u32 flags);

void push_clear(CommandBuffer* buffer, V3 color);

void push_cube(RenderGroup* group, V3 pos, V3 radius, TextureHandle texture, V3 color);
void push_model(RenderGroup* group, ModelHandle handle, V3 pos, V3 scale);
void push_rigged_model(RenderGroup* group, RiggedModelHandle* handle, Mat4* pose, V3 pos, V3 scale);
void push_debug_pose(RenderGroup* group, Skeleton* sk, Mat4* pose, V3 pos, V3 scale);
void push_line(RenderGroup* group, V3 start, V3 end, V3 color);

void push_spotlight(CommandBuffer* buffer, V3 pos, V3 dir, float fov, float far_plane);

TextureLoadOp texture_load_op(TextureHandle* handle, const char* path);
void free_texture_load_op(TextureLoadOp* load_op);
ModelLoadOp model_load_op(ModelHandle* handle, const char* path, Arena* tmp);
ModelLoadOp sk_model_load_op(RiggedModelHandle* handle, const char* path, Arena* tmp, Arena* assets); 

inline bool equal_settings(RenderSettings* a, RenderSettings* b) 
{
    return (a->width == b->width) && (a->height == b->height);
}


enum KeyType
{
    KeyType_Pos,
    KeyType_Rot,
    KeyType_Scale,
};

struct AnimationKey
{
    float timestamp;
    KeyType type;

    union
    {
        V3 v3;
        Quat rot;
    };
};

struct AnimationNode
{
    Str name;
    Mat4 trans;
    u32 first_child;
    u32 child_count;
    // NOTE: -1 means there is no bone belonging to this node
    i32 bone;
};

struct Bone
{
    Str name;
    u32 key_offset;
    u32 key_count;
};

struct Animation
{
    u32 node_count;
    AnimationNode* node;

    u32 bone_count;
    Bone* bone;

    u32 key_count;
    AnimationKey* key;

    float duration;
    float tps;
};

Animation load_animation(const char* path, Arena* assets);

Mat4* default_pose(Skeleton* skeleton, Arena* arena);
Mat4* interpolate_pose(Animation* animation, Skeleton* skeleton, Arena* arena, float t);

#endif
