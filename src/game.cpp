
#include "include/entity.h"

#include "include/opengl_renderer.h"
#include "include/game_math.h"
#include "include/arena.h"

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








void add_entity_type(EntityType type, Game* game){
    game->types[game->type_count] = type;
    game->type_count++;
}

void game_init_entity_types(Game* game, TextureHandle* white_texture){
    game->type_count = 0;

    //Wall
    EntityType wall;
    wall.id = EntityType_Wall;
    wall.sizeof_entity = sizeof(ColliderEntity);
    wall.collideable = true;
    wall.init = &collider_entity_standard_init;
    wall.update = &entity_standard_update;
    wall.render = &entity_standard_render;
    wall.try_move_into = &static_try_move_into;
    wall.collision_response = &standard_collision_response;
    wall.texture = &wall_texture;
    wall.render_color = v3(1);

    wall.load_tile_red = 0;
    wall.load_tile_green = 0;
    wall.load_tile_blue = 0;

    add_entity_type(wall,game);

    //Crate
    EntityType crate;
    crate.id = EntityType_Crate;
    crate.sizeof_entity = sizeof(ColliderEntity);
    crate.collideable = true;
    crate.init = &collider_entity_standard_init;
    crate.update = &entity_standard_update;
    crate.render = &entity_standard_render;
    crate.try_move_into = &moveable_try_move_into;
    crate.collision_response = &moveable_collision_response;
    crate.texture = &crate_texture;
    crate.render_color = v3(1);

    crate.load_tile_red = 88;
    crate.load_tile_green = 57;
    crate.load_tile_blue = 39;
    
    add_entity_type(crate,game);
    
    //Objective
    EntityType objective;
    objective.id = EntityType_Objective;
    objective.sizeof_entity = sizeof(ObjectiveEntity);
    objective.collideable = true;
    objective.init = &objective_init;
    objective.update = &entity_standard_update;
    objective.render = &objective_render;
    objective.try_move_into = &noclip_try_move_into;
    objective.collision_response = &objective_collision_response;
    objective.texture = white_texture;
    objective.render_color = v3(0,1,0);

    objective.load_tile_red = 1;
    objective.load_tile_green = 125;
    objective.load_tile_blue = 10;
    
    add_entity_type(objective,game);


    
}


void EntityType::allocate_memory(Game* game, Arena* arena){
    entity_list = (Entity*) push_size(arena, sizeof_entity * cap);
}


void game_init(Game* game, Arena* arena, u32 stage)
{
    game->reset_stage = false;
    game->current_level = stage;

    game->camera_state = CameraState_Locked;

    char path[1024];
    sprintf(path, "assets/stages/%u.png", stage);
    u8* tmp = stbi_load(path, (i32*) &game->width, (i32*) &game->height, NULL, STBI_rgb);
    assert(tmp);

    for (u32 i=0;i<game->type_count;i++) {
        game->types[i].cap = 0;
        game->types[i].count = 0;
    }


    u8* curr = tmp;
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            for (int i=0; i < game->type_count; i++) {
                if (curr[0] == game->types[i].load_tile_red &&
                    curr[1] == game->types[i].load_tile_green &&
                    curr[2] == game->types[i].load_tile_blue) {
                    game->types[i].cap++;
                }
            }
            curr += 3;
        }
    }
    

    for (u32 i=0;i<game->type_count;i++) {
        game->types[i].allocate_memory(game, arena);
    }






    curr = tmp;
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            for (u32 i=0; i < game->type_count; i++) {
                if (curr[0] == game->types[i].load_tile_red &&
                    curr[1] == game->types[i].load_tile_green &&
                    curr[2] == game->types[i].load_tile_blue) {
                    game->types[i].get_entity(game->types[i].count)->type = &game->types[i];
                    game->types[i].init(game->types[i].get_entity(game->types[i].count), game, x, y);
                    game->types[i].count++;
                }
            }

            if (curr[0] == 255 && curr[1] == 0 && curr[2] == 0) {
                game->player.pos = v3(x, y, 1);
            }

            curr += 3;
        }
    }

    

    stbi_image_free(tmp);

    game_reset_camera(game);
}

void game_update(Game* game, u8 inputs, float delta)
{
    // Update all entities
    for (u32 i=0;i<game->type_count;i++) {
        for (u32 j=0;j<game->types[i].count; j++) {
            game->types[i].update(game->types[i].get_entity(j), game);
        }
    }


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

        ColliderEntity player_collider;
        player_collider.radius = v3(0.35, 0.35, 0.7);

        movement.x *= delta * 10;
        movement.y *= delta * 10;

        move_and_collide(aabb(&game->player.pos, &player_collider), v2(movement.x, 0), game);
        move_and_collide(aabb(&game->player.pos, &player_collider), v2(0, movement.y), game);
    }

    // Check level completion
    bool level_completed = true;
    int objective_index = get_entity_type_index(EntityType_Objective, game);
    for (u32 i = 0; i < game->types[objective_index].count; ++i) {
        if (! ((ObjectiveEntity*) game->types[objective_index].get_entity(i))->broken) {
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


    // Render Entities
    for (u32 i = 0; i < game->type_count; i++){
        for (u32 j = 0; j < game->types[i].count; j++){
            assert(game->types[0].render_color.x == 1);
            game->types[i].render(game->types[i].get_entity(j), game, group, dbg);
        }
    }



    // Render Player
    push_cube(group, game->player.pos, v3(0.35, 0.35, 0.7), group->commands->white, v3(0, 0, 1));
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
