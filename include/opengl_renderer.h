#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"

#define FRAMEBUFFER_INITIALIZED (1 << 0)
#define FRAMEBUFFER_MULTISAMPLED (1 << 1)
#define FRAMEBUFFER_FILTERED (1 << 2)
#define FRAMEBUFFER_DEPTH (1 << 3)
#define FRAMEBUFFER_COLOR (1 << 4)

struct ProgramBase
{
    u32 id;
};

struct DrawShader
{
    ProgramBase base;
    u32 proj;
    u32 spotlight_pos;
    u32 spotlight_dir;
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
};

struct OpenGLContext
{
    RenderSettings prev_settings;
    u32 vertex_buffer;

    Arena render_arena;
    ModelShader model_shader;
    DrawShader quad_shader;
    ProgramBase post_shader;

    Framebuffer main_framebuffer;
    Framebuffer post_framebuffer;

    u32 quad_vao;
    u32 post_vao;

    i32 max_samples;
};

void opengl_init();
void opengl_render_commands(CommandBuffer* buffer);

void opengl_load_texture(TextureLoadOp* load_op);
void opengl_load_model(ModelLoadOp* load_op);

#endif
