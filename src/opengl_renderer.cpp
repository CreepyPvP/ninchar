#include "include/opengl_renderer.h"

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "include/types.h"
#include "include/util.h"

OpenGLContext opengl;

void APIENTRY debug_output(GLenum source, 
                           GLenum type, 
                           u32 id, 
                           GLenum severity, 
                           GLsizei length, 
                           const char *message, 
                           const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return; 
    }

    printf("---------------\n");
    printf("Debug message (%u): %s\n", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             printf("Source: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("Source: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("Source: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     printf("Source: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER:           printf("Source: Other\n"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               printf("Type: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("Type: Undefined Behaviour\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         printf("Type: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         printf("Type: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              printf("Type: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          printf("Type: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           printf("Type: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               printf("Type: Other\n"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         printf("Severity: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       printf("Severity: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          printf("Severity: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("Severity: notification\n"); break;
    }
    printf("\n");
}

ProgramBase load_program(const char* vertex_file, const char* frag_file)
{
    begin_tmp(&opengl.render_arena);

    char info_log[512];
    i32 status;

    char* vertex_code = read_file(vertex_file, NULL, &opengl.render_arena);
    assert(vertex_code);
    u32 vertex_prog = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_prog, 1, &vertex_code, NULL);
    glCompileShader(vertex_prog);
    glGetShaderiv(vertex_prog, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(vertex_prog, 512, NULL, info_log);
        printf("Error compiling vertex shader: %s\n", info_log);
        assert(0);
    }

    char* frag_code = read_file(frag_file, NULL, &opengl.render_arena);
    assert(frag_code);
    u32 frag_prog = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_prog, 1, &frag_code, NULL);
    glCompileShader(frag_prog);
    glGetShaderiv(frag_prog, GL_COMPILE_STATUS, &status);
    if(!status) {
        glGetShaderInfoLog(frag_prog, 512, NULL, info_log);
        printf("Error compiling fragment shader: %s\n", info_log);
        assert(0);
    }

    ProgramBase shader;
    shader.id = glCreateProgram();
    glAttachShader(shader.id, vertex_prog);
    glAttachShader(shader.id, frag_prog);
    glLinkProgram(shader.id);
    glGetProgramiv(shader.id, GL_LINK_STATUS, &status);
    if(!status) {
        glGetProgramInfoLog(shader.id, 512, NULL, info_log);
        printf("Error linking shader: %s\n", info_log);
        assert(0);
    }

    glDeleteShader(vertex_prog);
    glDeleteShader(frag_prog);
    end_tmp(&opengl.render_arena);
    
    return shader;
}

DrawShader load_draw_program(const char* vertex_file, const char* frag_file)
{
    DrawShader shader;
    shader.base = load_program(vertex_file, frag_file);
    shader.proj = glGetUniformLocation(shader.base.id, "proj");
    shader.spotlight_pos = glGetUniformLocation(shader.base.id, "sl_pos");
    shader.spotlight_dir = glGetUniformLocation(shader.base.id, "sl_dir");
    return shader;
}

void destroy_framebuffer(Framebuffer* framebuffer)
{
    bool initialized = framebuffer->flags & FRAMEBUFFER_INITIALIZED;
    bool depth = framebuffer->flags & FRAMEBUFFER_DEPTH;
    bool color = framebuffer->flags & FRAMEBUFFER_COLOR;

    if (initialized) {
        glDeleteFramebuffers(1, &framebuffer->id);
    }
    if (color) {
        glDeleteTextures(1, &framebuffer->color);
    }
    if (depth) {
        glDeleteRenderbuffers(1, &framebuffer->depth);
    }
}

Framebuffer create_framebuffer(u32 width, u32 height, u32 flags)
{
    Framebuffer res;
    res.flags = flags | FRAMEBUFFER_INITIALIZED;
    glGenFramebuffers(1, &res.id);

    bool multisampled = flags & FRAMEBUFFER_MULTISAMPLED;
    bool filtered = flags & FRAMEBUFFER_FILTERED;
    bool depth = flags & FRAMEBUFFER_DEPTH;
    bool color = flags & FRAMEBUFFER_COLOR;

    glBindFramebuffer(GL_FRAMEBUFFER, res.id);

    if (color) {
        u32 filter = filtered? GL_LINEAR : GL_NEAREST;
        u32 slot = multisampled? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

        glGenTextures(1, &res.color);
        glBindTexture(slot, res.color);
        if (multisampled) {
            glTexImage2DMultisample(slot, opengl.max_samples, GL_RGBA, width, height, GL_TRUE);
        } else {
            glTexImage2D(slot, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(slot, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(slot, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(slot, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(slot, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, slot, res.color, 0);
        glBindTexture(slot, 0);
    }

    if (depth) {
        glGenRenderbuffers(1, &res.depth);
        glBindRenderbuffer(GL_RENDERBUFFER, res.depth);

        if (multisampled) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, opengl.max_samples, 
                                             GL_DEPTH_COMPONENT, width, height);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        }

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, res.depth);
    }
    u32 attachments[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

    return res;
}

void opengl_init()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        assert(0 && "Failed to load required extensions\n");
    }
    init_arena(&opengl.render_arena, &pool);

    glGetIntegerv(GL_MAX_SAMPLES, &opengl.max_samples);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(debug_output, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    opengl.prev_settings.width = 0;
    opengl.prev_settings.height = 0;
    opengl.main_framebuffer.flags = 0;
    opengl.post_framebuffer.flags = 0;

    u32 vaos[2];
    glGenVertexArrays(2, vaos);

    opengl.quad_vao = vaos[0];
    glBindVertexArray(opengl.quad_vao);

    u32 buffers[2];
    glGenBuffers(2, buffers);

    opengl.vertex_buffer = buffers[0];
    glBindBuffer(GL_ARRAY_BUFFER, opengl.vertex_buffer);

    // 0: pos
    // 1: uv
    // 2: norm
    // 3: color
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, norm));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));

    float quad_verts[] = {
        -1, -1,
        -1, 1,
        1, -1,
        1, 1
    };
    opengl.post_vao = vaos[1];
    glBindVertexArray(opengl.post_vao);
    u32 post_buffer = buffers[1];
    glBindBuffer(GL_ARRAY_BUFFER, post_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 2, quad_verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    opengl.post_shader = load_program("shader/post.vert", "shader/post.frag");
    opengl.quad_shader = load_draw_program("shader/draw.vert", "shader/draw.frag");
    opengl.model_shader.draw = load_draw_program("shader/model.vert", "shader/model.frag");
    opengl.model_shader.trans = glGetUniformLocation(opengl.model_shader.draw.base.id, "trans");
}

void apply_settings(RenderSettings* settings) 
{
    destroy_framebuffer(&opengl.main_framebuffer);
    destroy_framebuffer(&opengl.post_framebuffer);

    opengl.main_framebuffer = create_framebuffer(settings->width, settings->height, FRAMEBUFFER_MULTISAMPLED | 
                                                 FRAMEBUFFER_DEPTH | FRAMEBUFFER_COLOR);
    opengl.post_framebuffer = create_framebuffer(settings->width, settings->height, FRAMEBUFFER_COLOR);

    opengl.prev_settings = *settings;
}

void prepare_render_setup(RenderSetup* setup, DrawShader* shader)
{
    // TODO: Apply draw->setup.lit here
    if (setup->culling) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glUseProgram(shader->base.id);
    glUniformMatrix4fv(shader->proj, 1, GL_FALSE, 
                       &(setup->proj)[0][0]);

    if (setup->spotlight_count == 1) {
        glUniform3f(shader->spotlight_pos, setup->spotlights[0].pos.x,
                    setup->spotlights[0].pos.y, setup->spotlights[0].pos.z);
        glUniform4f(shader->spotlight_dir, setup->spotlights[0].dir.x,
                    setup->spotlights[0].dir.y, setup->spotlights[0].dir.z,
                    setup->spotlights[0].fov);
    }
}

void opengl_render_commands(CommandBuffer* buffer)
{
    RenderSettings settings = buffer->settings;
    if (!equal_settings(&settings, &opengl.prev_settings)) {
        apply_settings(&settings);
    }

    glViewport(0, 0, settings.width, settings.height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    glBindFramebuffer(GL_FRAMEBUFFER, opengl.main_framebuffer.id);
    glBindVertexArray(opengl.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, opengl.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * buffer->quad_count * 4, 
                 buffer->vert_buffer, GL_STREAM_DRAW);

    u32 offset = 0;
    while (offset < buffer->entry_size) {
        CommandEntryHeader* header = (CommandEntryHeader*) (buffer->entry_buffer + offset);

        switch (header->type) {
            case EntryType_Clear: {
                CommandEntryClear* clear = (CommandEntryClear*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntryClear);
                glClearColor(clear->color.x, clear->color.y, clear->color.z, 1);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            } break;

            case EntryType_DrawQuads: {
                CommandEntryDrawQuads* draw = (CommandEntryDrawQuads*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntryDrawQuads);

                glBindVertexArray(opengl.quad_vao);

                prepare_render_setup(&draw->setup, &opengl.quad_shader);

                // TODO: glMultiDraw and BindLess Texture
                for (u32 i = 0; i < draw->quad_count; ++i) {
                    u32 quad_offset = i + draw->quad_offset;
                    glBindTexture(GL_TEXTURE_2D, buffer->texture_buffer[quad_offset].id);
                    glDrawArrays(GL_TRIANGLE_STRIP, 4 * quad_offset, 4);
                }
            } break;


            case EntryType_DrawModel: {
                CommandEntryDrawModel* draw = (CommandEntryDrawModel*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntryDrawModel);

                glBindVertexArray(draw->model.id);

                prepare_render_setup(&draw->setup, &opengl.model_shader.draw);
                glUniformMatrix4fv(opengl.model_shader.trans, 1, GL_FALSE, 
                                   &(draw->trans)[0][0]);

                glDrawElements(GL_TRIANGLES, draw->model.index_count, GL_UNSIGNED_INT, (void*) 0);
            } break;

            default: {
                return;
            }
        }
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, opengl.main_framebuffer.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, opengl.post_framebuffer.id);
    glBlitFramebuffer(0, 0, settings.width, settings.height, 0, 0, settings.width, settings.height, 
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(opengl.post_vao);
    glUseProgram(opengl.post_shader.id);

    glBindTexture(GL_TEXTURE_2D, opengl.post_framebuffer.color);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void opengl_load_texture(TextureLoadOp* load_op)
{
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    u32 format = GL_RGBA;
    if (load_op->num_channels == 3) {
        format = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 
                 load_op->width, load_op->height, 0, format, 
                 GL_UNSIGNED_BYTE, load_op->data);

    glGenerateMipmap(GL_TEXTURE_2D);

    load_op->handle->id = texture;
}

void opengl_load_model(ModelLoadOp* load_op)
{
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 buffers[2];
    glGenBuffers(2, buffers);

    u32 vert_buffer = buffers[0];
    u32 index_buffer = buffers[1];

    glBindBuffer(GL_ARRAY_BUFFER, vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, load_op->vert_stride * load_op->vert_count, load_op->vert_buffer, 
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * load_op->index_count, load_op->index_buffer, 
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, load_op->vert_stride, 0);

    load_op->handle->id = vao;
    load_op->handle->index_count = load_op->index_count;
}

