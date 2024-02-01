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
};

void opengl_init();
void opengl_render_commands(CommandBuffer* buffer);

void opengl_load_texture(TextureLoadOp* load_op);

#endif
