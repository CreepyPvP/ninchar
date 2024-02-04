#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"

#define FRAMEBUFFER_MULTISAMPLED (1 << 0)
#define FRAMEBUFFER_FILTERED (1 << 1)
#define FRAMEBUFFER_DEPTH (1 << 2)

struct Program
{
    u32 id;
};

struct DrawShader
{
    Program base;
    u32 proj;
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
    u32 vertex_buffer;

    Arena render_arena;
    DrawShader draw_shader;
    Program post_shader;

    Framebuffer main_framebuffer;
    Framebuffer post_framebuffer;

    u32 draw_vao;
    u32 quad_vao;

    i32 max_samples;
};

void opengl_init(u32 width, u32 height);
void opengl_render_commands(CommandBuffer* buffer);

void opengl_load_texture(TextureLoadOp* load_op);

#endif
