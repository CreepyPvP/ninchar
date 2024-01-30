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
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) 
        return; 

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

Program load_program(const char* vertex_file, const char* frag_file)
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

    Program shader;
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

void opengl_init()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        assert(0 && "Failed to load required extensions\n");
    }
    init_arena(&opengl.render_arena, &pool);

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(debug_output, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    opengl.draw_shader.base = load_program("shader/draw.vert", "shader/draw.frag");
    opengl.draw_shader.proj = glGetUniformLocation(opengl.draw_shader.base.id, "proj");
}

void opengl_render_commands(CommandBuffer* buffer)
{
    u32 offset = 0;
    while (offset < buffer->entry_size) {
        CommandEntryHeader* header = (CommandEntryHeader*) (buffer->entry_buffer + offset);

        switch (header->type) {
            case EntryType_Clear: {
                CommandEntry_Clear* clear = (CommandEntry_Clear*) (buffer->entry_buffer + offset);
                offset += sizeof(CommandEntry_Clear);
                glClearColor(clear->color.x, clear->color.y, clear->color.z, 1);
                glClear(GL_COLOR_BUFFER_BIT);
            } break;
        }
    }
}
