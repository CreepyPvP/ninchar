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
#include "include/game_math.h"


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
    u32 vert_cap = 5000000;
    Vertex* vert_buffer = (Vertex*) push_size(&arena, vert_cap * sizeof(Vertex));

    init_camera(&camera, v3(2), v3(-1, 0, 0));

    Profiler profiler_renderer;
    Profiler profiler_opengl_backend;

    Mat4 projection = glm::perspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    u32 box_count = 100000;
    V3* boxpos = (V3*) push_size(&arena, sizeof(V3) * box_count);
    V3* boxcolor = (V3*) push_size(&arena, sizeof(V3) * box_count);
    for (u32 i = 0; i < box_count; ++i) {
        boxpos[i].x = halton(i, 2) * 300;
        boxpos[i].y = halton(i, 3) * 300;
        boxpos[i].z = halton(i, 5) * 300;
        boxcolor[i].x = halton(i + 1, 5);
        boxcolor[i].y = halton(i + 1, 3);
        boxcolor[i].z = halton(i + 1, 2);
    }

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
                             vert_cap, vert_buffer);

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

        start_timestamp(&profiler_renderer);
        for (u32 i = 0; i < box_count; ++i) {
            push_cube(&group, boxpos[i], v3(1), boxcolor[i]);
        }
        double render_duration = end_timestamp(&profiler_renderer);

        start_timestamp(&profiler_opengl_backend);
        opengl_render_commands(&cmd);
        double opengl_backend_duration = end_timestamp(&profiler_opengl_backend);
        printf("Renderer: %f, Backend %f ms\n", 
               render_duration * 1000, opengl_backend_duration * 1000);

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

