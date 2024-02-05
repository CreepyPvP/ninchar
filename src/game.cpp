#include "include/game.h"

#include "include/opengl_renderer.h"
#include "include/game_math.h"
#include "include/arena.h"
#include "include/util.h"

#include "include/stb_image.h"

TextureHandle ground_texture;
TextureHandle wall_texture;
TextureHandle crate_texture;

ModelHandle teapot;


// TODO: move asset loading to asset queue. Then game has no opengl dependency
void game_load_assets()
{
    Arena asset_arena;
    init_arena(&asset_arena, &pool);

    TextureLoadOp load_ground = texture_load_op(&ground_texture, "assets/ground.png");
    opengl_load_texture(&load_ground);
    free_texture_load_op(&load_ground);

    TextureLoadOp load_crate = texture_load_op(&crate_texture, "assets/crate.png");
    opengl_load_texture(&load_crate);
    free_texture_load_op(&load_crate);

    TextureLoadOp load_wall = texture_load_op(&wall_texture, "assets/wall.png");
    opengl_load_texture(&load_wall);
    free_texture_load_op(&load_wall);

    ModelLoadOp load_teapot = model_load_op(&teapot, "assets/teapot.obj", &asset_arena);
    opengl_load_model(&load_teapot);

    dispose(&asset_arena);
};

void game_init(Game* game, Arena* arena, u32 stage)
{
    game->reset_stage = false;
    game->current_level = stage;

    game->camera_state = CameraState_Locked;

    char path[1024];
    sprintf(path, "assets/stages/%u.png", stage);
    u8* tmp = stbi_load(path, (i32*) &game->width, (i32*) &game->height, NULL, STBI_rgb);
    assert(tmp);

    game->wall_cap = 0;
    game->wall_count = 0;
    game->crate_cap = 0;
    game->crate_count = 0;
    game->objective_cap = 0;
    game->objective_count = 0;
    game->enemy_cap = 0;
    game->enemy_count = 0;

    u8* curr = tmp;
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            if (curr[0] == 0 && curr[1] == 0 && curr[2] == 0) {
                ++game->wall_cap;
            }
            if (curr[0] == 88 && curr[1] == 57 && curr[2] == 39) {
                ++game->crate_cap;
            }
            if (curr[0] == 1 && curr[1] == 125) {
                ++game->objective_cap;
            }
            if (curr[0] == 0 && curr[1] == 255 && curr[2] == 0) {
                ++game->enemy_cap;
            }

            curr += 3;
        }
    }

    game->crate = (Crate*) push_size(arena, sizeof(Crate) * game->crate_cap);
    game->wall = (Wall*) push_size(arena, sizeof(Wall) * game->wall_cap);
    game->objective = (Objective*) push_size(arena, sizeof(Objective) * game->objective_cap);
    game->enemy = (Enemy*) push_size(arena, sizeof(Enemy) * game->enemy_cap);

    curr = tmp;
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            if (curr[0] == 0 && curr[1] == 0 && curr[2] == 0) {
                game->wall[game->wall_count].pos = v3(x, y, 1);
                game->wall[game->wall_count].collider.radius = v3(0.5);
                game->wall[game->wall_count].collider.type = ColliderType_Static;
                game->wall[game->wall_count].collider.extra_data = NULL;
                ++game->wall_count;
            }
            if (curr[0] == 88 && curr[1] == 57 && curr[2] == 39) {
                game->crate[game->crate_count].pos = v3(x, y, 1);
                game->crate[game->crate_count].collider.radius = v3(0.5);
                game->crate[game->crate_count].collider.type = ColliderType_Moveable;
                game->crate[game->crate_count].collider.extra_data = NULL;
                ++game->crate_count;
            }
            if (curr[0] == 255 && curr[1] == 0 && curr[2] == 0) {
                game->player.pos = v3(x, y, 1);
            }
            if (curr[0] == 1 && curr[1] == 125) {
                game->objective[game->objective_count].pos = v3(x, y, 1);
                game->objective[game->objective_count].collider.radius = v3(0.5);
                game->objective[game->objective_count].collider.type = ColliderType_Objective;
                game->objective[game->objective_count].collider.extra_data = &game->objective[game->objective_count];
                game->objective[game->objective_count].broken = false;
                ++game->objective_count;
            }
            if (curr[0] == 0 && curr[1] == 255 && curr[2] == 0) {
                game->enemy[game->enemy_count].pos = v3(x, y, 1);
                ++game->enemy_count;
            }

            curr += 3;
        }
    }

    stbi_image_free(tmp);

    game_reset_camera(game);
}

void game_update(Game* game, u8 inputs, float delta)
{
    // Update Player
    if (game->camera_state == CameraState_Free) {
        update_camera(&game->camera, inputs, delta);
    } else {

        V2 movement = v2(0);

        // w
        if (inputs & (1 << 0)) {
            movement.y += 1;
        }
        // s
        if (inputs & (1 << 1)) {
            movement.y -= 1;
        }
        // a
        if (inputs & (1 << 2)) {
            movement.x -= 1;
        }
        // d
        if (inputs & (1 << 3)) {
            movement.x += 1;
        }

        movement = norm(movement);

        Collider player_collider;
        player_collider.radius = v3(0.35, 0.35, 0.7);

        movement.x *= delta * 10;
        movement.y *= delta * 10;

        move_and_collide(aabb(&game->player.pos, &player_collider), v2(movement.x, 0), game);
        move_and_collide(aabb(&game->player.pos, &player_collider), v2(0, movement.y), game);
    }

    // Update Objective
    bool level_completed = true;
    for (u32 i = 0; i < game->objective_count; ++i) {
        if (!game->objective[i].broken){
            level_completed = false;
        }
    }
    if (level_completed){
        game->reset_stage = true;
        game->current_level = (game->current_level + 1) % game->total_level_count;
    }
}

void game_render(Game* game, RenderGroup* group, RenderGroup* dbg){

    // Render Ground
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            push_cube(group, v3(x, y, 0), v3(0.5), ground_texture, v3(1));
        }
    }

    // Render Crates
    for (u32 i = 0; i < game->crate_count; ++i) {
        push_cube(group, game->crate[i].pos, v3(0.5), crate_texture, v3(1));
    }

    // Render Walls
    for (u32 i = 0; i < game->wall_count; ++i) {
        push_cube(group, game->wall[i].pos, v3(0.5), wall_texture, v3(1));
    }

    // Render Enemies
    for (u32 i = 0; i < game->enemy_count; ++i) {
        push_model(dbg, teapot, game->enemy[i].pos, v3(0.5));
    }

    // Render Objectives
    for (u32 i = 0; i < game->objective_count; ++i) {
        if (!game->objective[i].broken){
            push_cube(group, game->objective[i].pos, v3(0.5), group->commands->white, v3(0, 1, 0));
        }
    }

    // Render Player
    push_cube(group, game->player.pos, v3(0.35, 0.35, 0.7), group->commands->white, v3(0, 0, 1));
    
    game_raycast(game, game->player.pos, v3(-1, 0, 0), dbg);
}

void game_reset_camera(Game* game)
{
    init_camera(&game->camera, v3(game->width / 2, game->height / 2, 15), v3(0, 0.01, -1));
}

void game_toggle_camera_state(Game* game)
{
    if (game->camera_state == CameraState_Free) {
        game->camera_state = CameraState_Locked;
        game_reset_camera(game);
    } else {
        game->camera_state = CameraState_Free;
    }
}

void game_raycast(Game* game, V3 origin, V3 dir, RenderGroup* dbg)
{
    float t;
    bool hit_found = false;
    V3 hit;
    FOR_POS_COLLIDER(game, {
        V3 chit;
        float ct;
        if (hit_bounding_box(origin, dir, *pos, collider->radius, &chit, &ct)) {
            if (!hit_found || ct < t) {
                hit_found = true;
                t = ct;
                hit = chit;
            }
        }
    });
    if (hit_found) {
        push_line(dbg, origin, hit, v3(1, 0, 0));
    }
}
