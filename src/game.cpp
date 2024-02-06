
#include "include/entity.h"

#include "include/opengl_renderer.h"
#include "include/game_math.h"
#include "include/arena.h"

#include "include/stb_image.h"

#include "include/util.h"

TextureHandle ground_texture;
TextureHandle wall_texture;
TextureHandle crate_texture;

ColorHandle green_color;

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

    green_color.color = v3(0,1,0);

    ModelLoadOp load_teapot = model_load_op(&teapot, "assets/teapot.obj", &asset_arena);
    opengl_load_model(&load_teapot);

    dispose(&asset_arena);
};








void add_entity_type(EntityType type, Game* game){
    game->types[game->type_count] = type;
    game->type_count++;
}

void game_init_entity_types(Game* game){
    game->type_count = 0;

    //Wall
    EntityType wall;
    wall.id = EntityType_Wall;
    wall.sizeof_entity = sizeof(ColliderEntity);
    wall.collideable = true;
    wall.init = &collider_entity_standard_init;
    wall.update = &entity_standard_update;
    wall.render = &entity_texture_render;
    wall.try_move_into = &static_try_move_into;
    wall.collision_response = &standard_collision_response;
    wall.render_data = &wall_texture;
    wall.always_transparent = false;

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
    crate.render = &entity_texture_render;
    crate.try_move_into = &moveable_try_move_into;
    crate.collision_response = &moveable_collision_response;
    crate.render_data = &crate_texture;
    crate.always_transparent = false;

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
    objective.render_data = &green_color;
    objective.always_transparent = false;

    objective.load_tile_red = 1;
    objective.load_tile_green = 125;
    objective.load_tile_blue = -1;
    
    add_entity_type(objective,game);

    //Player
    EntityType player;
    player.id = EntityType_Player;
    player.sizeof_entity = sizeof(PlayerEntity);
    player.collideable = true;
    player.init = &player_init;
    player.update = &player_update;
    player.render = &player_render;
    player.try_move_into = &moveable_try_move_into;
    player.collision_response = &moveable_collision_response;
    player.render_data = NULL;
    player.always_transparent = false;

    player.load_tile_red = 255;
    player.load_tile_green = 0;
    player.load_tile_blue = 0;

    add_entity_type(player, game);

    //Enemies
    EntityType enemy;
    enemy.id = EntityType_Enemy;
    enemy.sizeof_entity = sizeof(EnemyEntity);
    enemy.collideable = true;
    enemy.init = &enemy_init;
    enemy.update = &enemy_update;
    enemy.render = &enemy_render;
    enemy.try_move_into = &static_try_move_into;
    enemy.collision_response = &standard_collision_response;
    enemy.render_data = &crate_texture;
    enemy.always_transparent = true;
    
    enemy.load_tile_red = 0;
    enemy.load_tile_green = 255;
    enemy.load_tile_blue = 0;

    add_entity_type(enemy, game);

    //Glass Wall
    
    EntityType glass_wall;
    glass_wall.id = EntityType_Wall;
    glass_wall.sizeof_entity = sizeof(ColliderEntity);
    glass_wall.collideable = true;
    glass_wall.init = &collider_entity_standard_init;
    glass_wall.update = &entity_standard_update;
    glass_wall.render = &entity_glass_wall_render;
    glass_wall.try_move_into = &static_try_move_into;
    glass_wall.collision_response = &standard_collision_response;
    glass_wall.render_data = NULL;
    glass_wall.always_transparent = true;

    glass_wall.load_tile_red = 208;
    glass_wall.load_tile_green = 251;
    glass_wall.load_tile_blue = 255;

    add_entity_type(glass_wall,game);

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
                if ((game->types[i].load_tile_red == -1 || curr[0] == game->types[i].load_tile_red) &&
                    (game->types[i].load_tile_green == -1 || curr[1] == game->types[i].load_tile_green) &&
                    (game->types[i].load_tile_blue == -1 || curr[2] == game->types[i].load_tile_blue)) {
                    game->types[i].cap++;
                }
            }
            curr += 3;
        }
    }
    

    for (u32 i=0;i<game->type_count;i++) {
        game->types[i].set_entity_list((Entity*) push_size(arena, game->types[i].sizeof_entity * game->types[i].cap));
    }






    curr = tmp;
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            for (u32 i=0; i < game->type_count; i++) {
                if ((game->types[i].load_tile_red == -1 || curr[0] == game->types[i].load_tile_red) &&
                    (game->types[i].load_tile_green == -1 || curr[1] == game->types[i].load_tile_green) &&
                    (game->types[i].load_tile_blue == -1 || curr[2] == game->types[i].load_tile_blue)) {
                    game->types[i].get_entity(game->types[i].count)->type = &game->types[i];
                    game->types[i].init(game->types[i].get_entity(game->types[i].count), game, x, y, curr[0], curr[1], curr[2]);
                    game->types[i].count++;
                }
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
            game->types[i].update(game->types[i].get_entity(j), game, inputs, delta);
        }
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
            game->types[i].render(game->types[i].get_entity(j), game, group, dbg);
        }
    }
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




RaycastResult game_raycast(Game* game, V3 origin, V3 dir){
    float t;
    bool hit_found = false;
    V3 hit;
    ColliderEntity* hit_entity;
    
    for (u32 i = 0; i < game->type_count; i++){ 
        if (game->types[i].collideable && !game->types[i].always_transparent){  
            for (u32 j = 0; j < game->types[i].count; j++){  
                V3* pos = &game->types[i].get_entity(j)->pos;   
                ColliderEntity* entity = (ColliderEntity*)(game->types[i].get_entity(j));  
                V3 chit;
                float ct;
                if (!entity->transparent && hit_bounding_box(origin, dir, *pos, entity->radius, &chit, &ct)) {
                    if (!hit_found || ct < t) {
                        hit_found = true;
                        t = ct;
                        hit = chit;
                        hit_entity = entity;
                    }
                }
            }
        }
    }
    return {t, hit_found, hit, hit_entity};
}

void game_render_raycast(Game* game, V3 origin, V3 dir, RenderGroup* dbg)
{
    RaycastResult res = game_raycast(game, origin, dir);
    if (res.hit_found) {
        push_line(dbg, origin, res.hit, v3(1, 0, 0));
    }
}