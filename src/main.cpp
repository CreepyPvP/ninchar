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
#include "include/camera.h"
#include "include/profiler.h"


struct GameWindow {
    GLFWwindow* handle;
    u32 width;
    u32 height;
};

GameWindow global_window;
Camera camera;

double last_mouse_pos_x;
double last_mouse_pos_y;


void mouse_callback(GLFWwindow* window, double pos_x, double pos_y) 
{
    float x_offset = pos_x - last_mouse_pos_x;
    float y_offset = pos_y - last_mouse_pos_y;
    update_camera_mouse(&camera, x_offset * 0.1, y_offset * 0.1);

    last_mouse_pos_x = pos_x;
    last_mouse_pos_y = pos_y;
}

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
    glfwSetCursorPosCallback(global_window.handle, mouse_callback);
    glfwSetInputMode(global_window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    u32 vert_cap = 500000;
    Vertex* vert_buffer = (Vertex*) push_size(&arena, vert_cap * sizeof(Vertex));
    u32 index_cap = 600000;
    u32* index_buffer = (u32*) push_size(&arena, index_cap * sizeof(u32));

    init_camera(&camera, v3(2), v3(-1, 0, 0));

    Profiler profiler_main;

    Mat4 projection = glm::perspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    while (!glfwWindowShouldClose(global_window.handle)) {
        if (glfwGetKey(global_window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(global_window.handle, true);
        }

        u8 pressed = (glfwGetKey(global_window.handle, GLFW_KEY_W) == GLFW_PRESS) << 0 |
                     (glfwGetKey(global_window.handle, GLFW_KEY_S) == GLFW_PRESS) << 1 |
                     (glfwGetKey(global_window.handle, GLFW_KEY_A) == GLFW_PRESS) << 2 |
                     (glfwGetKey(global_window.handle, GLFW_KEY_D) == GLFW_PRESS) << 3;
        update_camera(&camera, pressed, 1.0f / 60.0f);

        if (glfwGetKey(global_window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(global_window.handle, true);
        }

        cmd = command_buffer(entry_size, entry_buffer,
                             vert_cap, vert_buffer,
                             index_cap, index_buffer);

        Mat4 view = glm::lookAt(
            glm::vec3(camera.pos.x, camera.pos.y, camera.pos.z),
            glm::vec3(camera.pos.x + camera.front.x, 
                      camera.pos.y + camera.front.y, 
                      camera.pos.z + camera.front.z),
            glm::vec3(0, 0, 1)
        );
        Mat4 proj = projection * view;

        push_clear(&cmd, v3(0.1, 0.1, 0.2));
        RenderGroup group = render_group(&cmd, proj);

        start_timestamp(&profiler_main);

        for (u32 i = 0; i < 1; ++i) {
            push_cube(&group, v3(0), v3(1), v3(1));
        }

        double duration = end_timestamp(&profiler_main);
        opengl_render_commands(&cmd);

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

