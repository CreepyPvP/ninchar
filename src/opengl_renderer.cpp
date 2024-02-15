#include "include/opengl_renderer.h"

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "include/types.h"
#include "include/util.h"
#include "include/profiler.h"

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
    shader.light_space = glGetUniformLocation(shader.base.id, "sl_light_space");
    shader.spotlight_count = glGetUniformLocation(shader.base.id, "sl_count");
    shader.spotlight_pos = glGetUniformLocation(shader.base.id, "sl_pos");
    shader.spotlight_dir = glGetUniformLocation(shader.base.id, "sl_dir");
    shader.spotlight_fov = glGetUniformLocation(shader.base.id, "sl_fov");
    
    shader.shadow_map = glGetUniformLocation(shader.base.id, "sl_shadowmap");

    return shader;
}

void destroy_framebuffer(Framebuffer* framebuffer)
{
    bool initialized = framebuffer->flags & FRAMEBUFFER_INITIALIZED;
    bool depth = framebuffer->flags & FRAMEBUFFER_DEPTH;
    bool color = framebuffer->flags & FRAMEBUFFER_COLOR;
    bool depth_tex = framebuffer->flags & FRAMEBUFFER_DEPTH_TEX;

    if (initialized) {
        glDeleteFramebuffers(1, &framebuffer->id);
    }
    if (color) {
        glDeleteTextures(1, &framebuffer->color);
    }
    if (depth_tex) {
        glDeleteTextures(1, &framebuffer->depth_tex);
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
    bool depth_tex = flags & FRAMEBUFFER_DEPTH_TEX;
    bool color = flags & FRAMEBUFFER_COLOR;

    glBindFramebuffer(GL_FRAMEBUFFER, res.id);

    u32 filter = filtered? GL_LINEAR : GL_NEAREST;
    if (color) {
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
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
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
    if (depth_tex) {
        glGenTextures(1, &res.depth_tex);
        glBindTexture(GL_TEXTURE_2D, res.depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, res.depth_tex, 0);
    }

    u32 attachments[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

    return res;
}

void set_uniform_mat4(u32 id, Mat4* mat, u32 count)
{
    glUniformMatrix4fv(id, count, GL_FALSE, (GLfloat*) mat);
}

void opengl_init()
{

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        assert(0 && "Failed to load required extensions\n");
    }
    opengl = {};
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
    // 4: texture
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, norm));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 2, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, texture));

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

    opengl.shadow_shader.base = load_program("shader/shadow.vert", "shader/shadow.frag");
    opengl.shadow_shader.light_space = glGetUniformLocation(opengl.shadow_shader.base.id, "light_space");

    for (u32 i = 0; i < SHADOW_MAP_COUNT; ++i) {
        opengl.shadow_maps[i] = create_framebuffer(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 
                                                   FRAMEBUFFER_DEPTH_TEX);

        opengl.shadow_map_handles[i] = glGetTextureHandleARB(opengl.shadow_maps[i].depth_tex);
        glMakeTextureHandleResidentARB(opengl.shadow_map_handles[i]);
    }
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

void prepare_render_setup(RenderSetup* setup, DrawShader* shader, SpotLight* lights, u32 light_count)
{
    // TODO: Apply draw->setup.lit here
    if (setup->culling) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glUseProgram(shader->base.id);
    set_uniform_mat4(shader->proj, &setup->proj, 1);

    if (setup->lit) {
        V3 pos_acc[MAX_SPOTLIGHTS];
        V3 dir_acc[MAX_SPOTLIGHTS];
        float fov_acc[MAX_SPOTLIGHTS];
        Mat4 light_space_acc[MAX_SPOTLIGHTS];
        u64 shadow_map_acc[MAX_SPOTLIGHTS];

        glUniform1ui(shader->spotlight_count, light_count);

        for (u32 i = 0; i < light_count; ++i) {
            light_space_acc[i] = lights[i].light_space;
            shadow_map_acc[i] = opengl.shadow_map_handles[lights[i].shadow_map];
            pos_acc[i] = lights[i].pos;
            dir_acc[i] = lights[i].dir;
            fov_acc[i] = lights[i].fov;

        }

        set_uniform_mat4(shader->light_space, light_space_acc, light_count);
        glUniform2uiv(shader->shadow_map, light_count, (u32*) shadow_map_acc);
        glUniform3fv(shader->spotlight_pos, light_count, (float*) pos_acc);
        glUniform3fv(shader->spotlight_dir, light_count, (float*) dir_acc);
        glUniform1fv(shader->spotlight_fov, light_count, (float*) fov_acc);
    }
}

void draw_quads(CommandEntryDrawQuads* draw)
{
    begin_tmp(&opengl.render_arena);
    i32* first = (i32*) push_size(&opengl.render_arena, sizeof(i32) * draw->quad_count);
    i32* count = (i32*) push_size(&opengl.render_arena, sizeof(i32) * draw->quad_count);

    for (u32 i = 0; i < draw->quad_count; ++i) {
        first[i] = draw->vert_offset + 4 * i;
        count[i] = 4;
    }

    glMultiDrawArrays(GL_TRIANGLE_STRIP, first, count, draw->quad_count);
    end_tmp(&opengl.render_arena);
}

void do_shadowpass(CommandBuffer* buffer, SpotLight* light)
{
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, opengl.shadow_maps[light->shadow_map].id);
    glUseProgram(opengl.shadow_shader.base.id);
    set_uniform_mat4(opengl.shadow_shader.light_space, &light->light_space, 1);

    glClear(GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(opengl.quad_vao);

    u32 offset = 0;

    while (offset < buffer->entry_size) {
        CommandEntryHeader* header = (CommandEntryHeader*) (buffer->entry_buffer + offset);

        switch(header->type) {
            case EntryType_Clear: {
                offset += sizeof(CommandEntryClear);
            } break;

            case EntryType_PushLight: {
                offset += sizeof(CommandEntryPushLight);
            } break;

            case EntryType_DrawQuads: {
                CommandEntryDrawQuads* draw = (CommandEntryDrawQuads*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntryDrawQuads);

                if (draw->setup.shadow_caster) {
                    draw_quads(draw);
                }
            } break;

            case EntryType_DrawModel: {
                offset += sizeof(CommandEntryDrawModel);
            } break;
        }
    }
}

void opengl_render_commands(CommandBuffer* buffer)
{
    LogEntryInfo info = start_log(LogTarget_Backend);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * buffer->vert_count, 
                 buffer->vert_buffer, GL_STREAM_DRAW);

    u32 light_count = 0;
    u32 shadow_map_count = 0;
    SpotLight lights[MAX_SPOTLIGHTS];

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

                prepare_render_setup(&draw->setup, &opengl.quad_shader, lights, light_count);

                draw_quads(draw);
            } break;

            case EntryType_DrawModel: {
                CommandEntryDrawModel* draw = (CommandEntryDrawModel*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntryDrawModel);

                Model* model = opengl.models + draw->model.id;

                prepare_render_setup(&draw->setup, &opengl.model_shader.draw, lights, light_count);
                set_uniform_mat4(opengl.model_shader.trans, &draw->trans, 1);

                for (u32 i = 0; i < model->mesh_count; ++i) {
                    Mesh* mesh = opengl.meshes + model->mesh_offset + i;

                    glBindVertexArray(mesh->vao);
                    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, (void*) 0);
                }

            } break;

            case EntryType_PushLight: {
                CommandEntryPushLight* light = (CommandEntryPushLight*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntryPushLight);

                assert(light_count < MAX_SPOTLIGHTS);
                lights[light_count].pos = light->pos;
                lights[light_count].dir = light->dir;
                lights[light_count].fov = light->fov;
                lights[light_count].light_space = light->light_space;

                assert(shadow_map_count < SHADOW_MAP_COUNT);
                lights[light_count].shadow_map = shadow_map_count;
                ++shadow_map_count;

                do_shadowpass(buffer, lights + light_count);
                ++light_count;

                glBindFramebuffer(GL_FRAMEBUFFER, opengl.main_framebuffer.id);
                glViewport(0, 0, settings.width, settings.height);
                glEnable(GL_CULL_FACE);
            } break;

            default: {
                end_log(info);
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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl.post_framebuffer.color);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    end_log(info);
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

    u64 handle = glGetTextureHandleARB(texture);
    glMakeTextureHandleResidentARB(handle);

    load_op->handle->id = handle;
}

void opengl_load_model(ModelLoadOp* load_op)
{
    assert(opengl.model_count < MODEL_CAP);
    Model model;
    model.mesh_offset = opengl.mesh_count;
    model.mesh_count = load_op->mesh_count;

    for (u32 i = 0; i < load_op->mesh_count; ++i) {
        MeshInfo* info = load_op->meshes + i;

        u32 vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        u32 buffers[2];
        glGenBuffers(2, buffers);

        u32 vert_buffer = buffers[0];
        u32 index_buffer = buffers[1];

        glBindBuffer(GL_ARRAY_BUFFER, vert_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * info->vertex_count, info->vertex_buffer, 
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * info->index_count, info->index_buffer, 
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*) offsetof(MeshVertex, uv));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*) offsetof(MeshVertex, norm));

        Mesh mesh = {};
        mesh.vao = vao;
        mesh.index_count = info->index_count;

        assert(opengl.mesh_count < MESH_CAP);
        opengl.meshes[opengl.mesh_count] = mesh;
        opengl.mesh_count++;
    }

    opengl.models[opengl.model_count] = model;
    load_op->handle->id = opengl.model_count;
    opengl.model_count++;
}

