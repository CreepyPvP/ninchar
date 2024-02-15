#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"

#define MESH_CAP 16
#define MODEL_CAP 8

#define SHADOW_MAP_COUNT 6
#define MAX_SPOTLIGHTS 6
#define SHADOW_MAP_SIZE 1024

#define FRAMEBUFFER_INITIALIZED (1 << 0)
#define FRAMEBUFFER_MULTISAMPLED (1 << 1)
#define FRAMEBUFFER_FILTERED (1 << 2)
#define FRAMEBUFFER_DEPTH (1 << 3)
#define FRAMEBUFFER_COLOR (1 << 4)
#define FRAMEBUFFER_DEPTH_TEX (1 << 5)


struct Mesh
{
    u32 vao;
    u32 index_count;
};

struct Model
{
    u32 mesh_offset;
    u32 mesh_count;
};

struct SpotLight
{
    V3 pos;
    V3 dir;
    float fov;

    Mat4 light_space;

    u32 shadow_map;
};


struct ProgramBase
{
    u32 id;
};

struct DrawShader
{
    ProgramBase base;
    u32 proj;

    u32 camera_pos;

    u32 spotlight_count;
    u32 spotlight_pos;
    u32 spotlight_dir;
    u32 spotlight_fov;
    u32 light_space;

    u32 shadow_map;
};

struct ShadowShader
{
    ProgramBase base;
    u32 light_space;
};

struct ModelShader
{
    DrawShader draw;
    u32 trans;
};

struct Framebuffer
{
    u32 id;
    u32 flags;

    u32 color;
    u32 depth;
    u32 depth_tex;
};

struct OpenGLContext
{
    RenderSettings prev_settings;
    u32 vertex_buffer;

    Arena render_arena;
    ModelShader model_shader;
    DrawShader quad_shader;
    ProgramBase post_shader;
    ShadowShader shadow_shader;

    Framebuffer main_framebuffer;
    Framebuffer post_framebuffer;

    u32 quad_vao;
    u32 post_vao;

    Framebuffer shadow_maps[SHADOW_MAP_COUNT];
    u64 shadow_map_handles[SHADOW_MAP_COUNT];

    u32 mesh_count;
    Mesh meshes[MESH_CAP];
    u32 model_count;
    Model models[MODEL_CAP];

    i32 max_samples;
};

void opengl_init();
void opengl_render_commands(CommandBuffer* buffer);

void opengl_load_texture(TextureLoadOp* load_op);
void opengl_load_model(ModelLoadOp* load_op);

#endif
