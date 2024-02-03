#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"

struct Program
{
    u32 id;
};

struct DrawShader
{
    Program base;
    u32 proj;
};

struct OpenGLContext
{
    u32 vertex_buffer;

    Arena render_arena;
    DrawShader draw_shader;
    Program post_shader;

    u32 main_framebuffer;
    u32 main_color;
    u32 main_depth;

    u32 draw_vao;
    u32 quad_vao;
};

void opengl_init(u32 width, u32 height);
void opengl_render_commands(CommandBuffer* buffer);

void opengl_load_texture(TextureLoadOp* load_op);

#endif
