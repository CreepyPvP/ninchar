#include <stdio.h>
#include <string>


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
#include "include/game.h"

struct GameWindow {
    GLFWwindow* handle;
    u32 width;
    u32 height;
};

GameWindow global_window;

double last_mouse_pos_x;
double last_mouse_pos_y;

Game game;

u32 level_count = 13;

std::string levels[] = {
    "1_1",
    "2_1",
    "2_2",
    "2_3",
    "2_4",
    "3_1",
    "4_1",
    "4_2",
    "5_1",
    "align_crates",
    "align_crates_2",
    "corner",
    "dont_destroy",
    "down_then_up",
    "jam_level",
    "jam_level_2",
    "many_objectives",
    "reflection_test",
    "reflections",
    "renderer_test",
    "down_then_up_glass",
    "4_1"
};

void resize_callback(GLFWwindow* window, i32 width, i32 height) 
{
    global_window.width = width;
    global_window.height = height;
}

void mouse_callback(GLFWwindow* window, double pos_x, double pos_y) 
{
    float x_offset = pos_x - last_mouse_pos_x;
    float y_offset = pos_y - last_mouse_pos_y;
    update_camera_mouse(&game.camera, x_offset * 0.1, y_offset * 0.1);

    last_mouse_pos_x = pos_x;
    last_mouse_pos_y = pos_y;
}

void create_window() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    glfwSetFramebufferSizeCallback(global_window.handle, resize_callback);
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
    u32 vert_cap = 100000;
    Vertex* vert_buffer = (Vertex*) push_size(&arena, vert_cap * sizeof(Vertex));

    TextureHandle white;
    TextureLoadOp load_white = texture_load_op(&white, "assets/white.png");
    opengl_load_texture(&load_white);
    free_texture_load_op(&load_white);

    Mat4 projection = glm::perspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    Arena game_arena;
    init_arena(&game_arena, &pool);
    game_load_assets();
    game = {};

    u32 current_level = 0;
    game_init(&game, &game_arena, levels[current_level], white);

    while (!glfwWindowShouldClose(global_window.handle)) {
        start_frame();

        if (glfwGetKey(global_window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(global_window.handle, true);
        }

#ifdef DEBUG
        static bool c_pressed = false;
        if (glfwGetKey(global_window.handle, GLFW_KEY_C) == GLFW_PRESS) {
            if (!c_pressed) {
                game_toggle_camera_state(&game);
            }
            c_pressed = true;
        } else {
            c_pressed = false;
        }

        static bool r_pressed = false;
        if (glfwGetKey(global_window.handle, GLFW_KEY_R) == GLFW_PRESS) {
            if (!r_pressed) {
                game.reset_stage = true;
            }
            r_pressed = true;
        } else {
            r_pressed = false;
        }

        static bool n_pressed = false;
        if (glfwGetKey(global_window.handle, GLFW_KEY_N) == GLFW_PRESS) {
            if (!n_pressed) {
                current_level = (current_level + 1) % level_count;
                game.reset_stage = true;
            }
            n_pressed = true;
        } else {
            n_pressed = false;
        }
#endif

        if (game.next_stage) {
            current_level = (current_level + 1) % level_count;
            game.reset_stage = true;
        }
        if (game.reset_stage){
            dispose(&game_arena);
            game = {};
            game_init(&game, &game_arena, levels[current_level], white);
        }


        u8 pressed = (glfwGetKey(global_window.handle, GLFW_KEY_W) == GLFW_PRESS) << 0 |
                     (glfwGetKey(global_window.handle, GLFW_KEY_S) == GLFW_PRESS) << 1 |
                     (glfwGetKey(global_window.handle, GLFW_KEY_A) == GLFW_PRESS) << 2 |
                     (glfwGetKey(global_window.handle, GLFW_KEY_D) == GLFW_PRESS) << 3;

        // printf("Camera pos: %f, %f, %f, Camera forward: %f %f %f\n", 
        //        game.camera.pos.x, game.camera.pos.y, game.camera.pos.z,
        //        game.camera.front.x, game.camera.front.y, game.camera.front.z);

        if (glfwGetKey(global_window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(global_window.handle, true);
        }

        cmd = command_buffer(entry_size, entry_buffer, 
                             vert_cap, vert_buffer, 
                             global_window.width, global_window.height, white);

        Mat4 view = glm::lookAt(
            glm::vec3(game.camera.pos.x, game.camera.pos.y, game.camera.pos.z),
            glm::vec3(game.camera.pos.x + game.camera.front.x, 
                      game.camera.pos.y + game.camera.front.y, 
                      game.camera.pos.z + game.camera.front.z),
            glm::vec3(0, 0, 1)
        );
        Mat4 proj = projection * view;

        push_clear(&cmd, v3(0.1, 0.1, 0.2));

        RenderGroup main_group = render_group(&cmd, proj, true, true, true);
        RenderGroup transparent_group = render_group(&cmd, proj, true, true, false);
        RenderGroup debug_group = render_group(&cmd, proj, false, false, false);

        game_update(&game, pressed, 1.0f / 60.0f, &main_group, &debug_group);
        game_render(&game, &main_group, &transparent_group, &debug_group);

        opengl_render_commands(&cmd);

        end_frame();

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

