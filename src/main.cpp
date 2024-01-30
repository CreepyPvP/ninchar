#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"
#include "include/opengl_renderer.h"
#include "include/profiler.h"

struct GameWindow {
    GLFWwindow* handle;
    u32 width;
    u32 height;
};


GameWindow global_window;

void create_window() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

#ifdef FULLSCREEN
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    global_window.width = 1920;
    global_window.height = 1080;
#else
    GLFWmonitor* monitor = NULL;
    global_window.width = 960;
    global_window.height = 540;
#endif
    global_window.handle = glfwCreateWindow(global_window.width, global_window.height, "Game", 
                                           monitor, NULL);
    if (!global_window.handle) {
        assert(0 && "Failed to create window\n");
    }

    // glfwSetFramebufferSizeCallback(Window.Handle, resize_cb);
    glfwMakeContextCurrent(global_window.handle);
}

i32 main()
{
    create_window();
    init_pool(&pool);

    opengl_init();

    Arena arena;
    init_arena(&arena, &pool);

    CommandBuffer cmd;
    u32 entry_size = 10000;
    u8* entry_buffer = (u8*) push_size(&arena, entry_size);
    u32 vert_cap = 10000;
    Vertex* vert_buffer = (Vertex*) push_size(&arena, vert_cap * sizeof(Vertex));
    u32 index_cap = 10000;
    u32* index_buffer = (u32*) push_size(&arena, index_cap * sizeof(u32));

    Profiler profiler_main;

    Mat4 projection = glm::ortho(
        -960.0f / 2,
        960.0f / 2,
        -520.0f / 2,
        520.0f / 2,
        .1f,
        1000.0f
    );
    Mat4 view = glm::lookAt(
        glm::vec3(0, 0, 100),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    Mat4 proj = projection * view;

    while (!glfwWindowShouldClose(global_window.handle)) {
        if (glfwGetKey(global_window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(global_window.handle, true);
        }

        start_timestamp(&profiler_main);

        cmd = command_buffer(entry_size, entry_buffer,
                             vert_cap, vert_buffer,
                             index_cap, index_buffer,
                             proj);

        push_clear(&cmd, v3(0.1, 0.1, 0.2));
        push_quad(&cmd, v2(-100), v2(100), v3(1));

        opengl_render_commands(&cmd);

        double duration = end_timestamp(&profiler_main);
        printf("Main loop took %f ms\n", duration * 1000);

        glfwSwapBuffers(global_window.handle);
        glfwPollEvents();
    }

    return 0;
}

#ifdef WINDOWS
i32 WinMain() {
    return main();
}
#endif

