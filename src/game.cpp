#include "include/game.h"

#include "include/opengl_renderer.h"
#include "include/game_math.h"
#include "include/arena.h"
#include "include/util.h"

#include "include/stb_image.h"

TextureHandle ground_texture;
TextureHandle wall_texture;
TextureHandle glass_wall_texture;
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

    TextureLoadOp load_glass_wall = texture_load_op(&glass_wall_texture, "assets/glasswall.png");
    opengl_load_texture(&load_glass_wall);
    free_texture_load_op(&load_glass_wall);

    ModelLoadOp load_teapot = model_load_op(&teapot, "assets/teapot.obj", &asset_arena);
    opengl_load_model(&load_teapot);

    dispose(&asset_arena);
};

void game_init(Game* game, Arena* arena, u32 stage, TextureHandle white)
{
    char path[1024];
    sprintf(path, "assets/stages/%u.png", stage);
    u8* tmp = stbi_load(path, (i32*) &game->width, (i32*) &game->height, NULL, STBI_rgb);
    assert(tmp);

    game->entities = (Entity*) push_size(arena, sizeof(Entity) * ENTITY_CAP);

    // TODO: Clean this up some more
    u8* curr = tmp;
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            Entity entity = {};
            entity.pos = v3(x, y, 1);
            entity.collider.radius = v3(0.5);
            entity.color = v3(1);
            entity.texture = white;
            entity.collider.transparency_type = TransparencyType_Opaque;
            entity.collider.camouflage_color = 0;

            if (curr[0] == 0 && curr[1] == 0 && curr[2] == 0) {
                entity.type = EntityType_Wall;
                entity.collider.type = ColliderType_Static;
                entity.texture = wall_texture;
                push_entity(entity, game);
            }

            if (curr[0] == 50 && curr[1] == 50 && curr[2] == 5) {
                entity.type = EntityType_GlassWall;
                entity.collider.type = ColliderType_Static;
                entity.collider.transparency_type = TransparencyType_Transparent;
                entity.texture = glass_wall_texture;
                push_entity(entity, game);
            }

            if (curr[0] == 88 && curr[1] == 57 && curr[2] == 39) {
                entity.type = EntityType_Crate;
                entity.collider.type = ColliderType_Moveable;
                entity.texture = crate_texture;
                push_entity(entity, game);
            }

            if (curr[0] == 255 && curr[1] == 0 && curr[2] == 0) {
                entity.type = EntityType_Player;
                entity.collider.type = ColliderType_Moveable;
                entity.collider.transparency_type = TransparencyType_Camouflage;
                entity.collider.camouflage_color = 1;
                entity.collider.radius = v3(0.35, 0.35, 0.7);
                entity.color = v3(0, 0, 1);
                game->player = push_entity(entity, game);
            }

            if (curr[0] == 1 && curr[1] == 125) {
                entity.type = EntityType_Objective;
                entity.collider.type = ColliderType_None;
                entity.collider.transparency_type = TransparencyType_Opaque;
                entity.collider.camouflage_color = 0;
                entity.color = v3(0, 1, 0);
                push_entity(entity, game);
            }

            if (curr[0] == 0 && curr[1] == 255 && curr[2] == 0) {
                entity.type = EntityType_Enemy;
                entity.collider.type = ColliderType_Static;
                entity.collider.transparency_type = TransparencyType_Opaque;
                entity.rotation = 1.2;
                push_entity(entity, game);
            }

            if (curr[0] == 195 && curr[1] == 195 && curr[2] == 195) {
                entity.type = EntityType_MirrorWall;
                entity.collider.type = ColliderType_Static;
                entity.collider.transparency_type = TransparencyType_Mirror;
                entity.texture = wall_texture;
                entity.color = v3(0.5, 0.5, 0.5);
                push_entity(entity, game);
            }

            if (curr[0] == 50 && curr[1] == 50 && curr[2] == 209) {
                entity.type = EntityType_ColoredWall;
                entity.collider.type = ColliderType_Static;
                entity.collider.transparency_type = TransparencyType_Opaque;
                entity.collider.camouflage_color = 1;
                entity.texture = wall_texture;
                entity.color = v3(0, 0, 1);
                push_entity(entity, game);
            }

            curr += 3;
        }
    }

    stbi_image_free(tmp);

    game_reset_camera(game);
}

void push_entity_to_list(EntityList* list, EntityRef ref)
{
    assert(list->entity_count < ACCESS_ENITTY_CAP);
    list->entity_refs[list->entity_count] = ref;
    ++list->entity_count;
}

EntityRef push_entity(Entity entity, Game* game)
{
    assert(game->entity_count < ENTITY_CAP);
    game->entities[game->entity_count] = entity;
    EntityRef ref;
    ref.id = game->entity_count;
    ++game->entity_count;

    if (entity.type == EntityType_Enemy) {
        push_entity_to_list(&game->enemies, ref);
    }
    if (entity.type == EntityType_Objective) {
        push_entity_to_list(&game->objectives, ref);
    }

    return ref;
}

Entity* get_entity(EntityRef ref, Game* game)
{
    return game->entities + ref.id;
}

void game_update(Game* game, u8 inputs, float delta, RenderGroup* group, RenderGroup* dbg)
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
        movement.x *= delta * 10;
        movement.y *= delta * 10;

        Entity* player = get_entity(game->player, game);
        move_and_collide(player, v2(movement.x, 0), game);
        move_and_collide(player, v2(0, movement.y), game);
    }

    for (u32 i = 0; i < game->enemies.entity_count; ++i) {
        Entity* enemy = get_entity(game->enemies.entity_refs[i], game);
        V3 facing = v3(sin(enemy->rotation), cos(enemy->rotation), 0);
        V3 side = v3(-facing.y, facing.x, facing.z);

        float fov = 0.275;
        V3 left = v3(fov * side.x + (1 - fov) * facing.x, fov * side.y + (1 - fov) * facing.y, facing.z);
        V3 right = v3(-fov * side.x + (1 - fov) * facing.x, -fov * side.y + (1 - fov) * facing.y, facing.z);

        //Pushing multiple spotlights seems to currently cause an error.
        if(i==0){
            //push_spotlight(group->commands, enemy->pos, facing, fov);
        }

        bool enemy_use_many_rays = true;

        if (enemy_use_many_rays){
            u32 ENEMY_RAY_COUNT = 40;
            float o = 2.0f / ENEMY_RAY_COUNT - 1;
            for (u32 j = 0; j < ENEMY_RAY_COUNT - 1; ++j) {
                o += 2.0f / ENEMY_RAY_COUNT;

                V3 r = v3((1 - fov) * facing.x + o * fov * side.x,
                    (1 - fov) * facing.y + o * fov * side.y, 0);
                RaycastResult raycast_result = game_raycast(game, enemy, enemy->pos, r, ENEMY_VISION, dbg);
                
                if (raycast_result.hit_found && raycast_result.final_hit_entity->type == EntityType_Player){
                    game->reset_stage = true;
                }
            }
        }else{
            //Old deprecated code for enemies seeing players.
            //Deprecated because it doesn't work with mirrors.
    #ifdef DEBUG
            game_raycast(game, enemy, enemy->pos, facing, ENEMY_VISION, dbg);
            game_raycast(game, enemy, enemy->pos, left, ENEMY_VISION, dbg);
            game_raycast(game, enemy, enemy->pos, right, ENEMY_VISION, dbg);
    #endif

            Entity* player = get_entity(game->player, game);
            V3 player_dir = v3(player->pos.x - enemy->pos.x, 
                           player->pos.y - enemy->pos.y, 
                           player->pos.z - enemy->pos.z);

            RaycastResult ray_res = game_raycast(game, enemy, enemy->pos, player_dir, ENEMY_VISION, dbg);
            if (ray_res.hit_found) {
                if (ray_res.final_hit_entity->type == EntityType_Player) {
                    if (dot(norm(player_dir), facing) > dot(norm(left), facing)) {
                        game->reset_stage = true;
                    }
                }
            }
        }

        enemy->rotation += 0.8 * delta;
    }

    // Update Objective
    bool level_completed = true;
    for (u32 i = 0; i < game->objectives.entity_count; ++i) {
        Entity* entity = get_entity(game->objectives.entity_refs[i], game);
        if (!entity->objective.broken) {
            level_completed = false;
        }
    }

    if (level_completed){
        game->next_stage = true;
    }
}

void game_render(Game* game, RenderGroup* group, RenderGroup* dbg){
    // Render Ground
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            push_cube(group, v3(x, y, 0), v3(0.5), ground_texture, v3(1));
        }
    }

    for (u32 i = 0; i < game->entity_count; ++i) {
        Entity* entity = game->entities + i;

        if (entity->type == EntityType_Enemy) {
            push_model(dbg, teapot, entity->pos, entity->collider.radius);
            continue;
        }

        if (entity->type == EntityType_Objective && entity->objective.broken) {
            continue;
        }

        push_cube(group, entity->pos, entity->collider.radius, entity->texture, entity->color);
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

RaycastResult game_raycast(Game* game, Entity* origin_entity, V3 origin, V3 dir, u32 mask, RenderGroup* dbg)
{
    RaycastResult res;
    res.hit_found = false;
    res.t = INFINITY;
    res.directly_hit_entity = NULL;
    res.final_hit_entity = NULL;
    
    for (u32 i = 0; i < game->entity_count; ++i) {
        Entity* entity = game->entities + i;

        if (entity->collider.transparency_type == TransparencyType_Transparent ||
            entity == origin_entity ||
          !(1 & (mask >> entity->type))) {
            continue;
        }

        V3 chit;
        float ct;
        if (hit_bounding_box(origin, dir, entity->pos, entity->collider.radius, &chit, &ct)) {
            if (!res.hit_found || ct < res.t) {
                res.hit_found = true;
                res.t = ct;
                res.hit_pos = chit;
                res.directly_hit_entity = entity;
                res.final_hit_entity = entity;
            }
        }
    }

    if(res.hit_found && res.directly_hit_entity->collider.transparency_type == TransparencyType_Mirror){
        Entity* mirror = res.directly_hit_entity;
        V3 mirrored_dir = dir;
        float precision = 0.0001f;
        bool mirror_x = abs(res.hit_pos.x - mirror->pos.x - mirror->collider.radius.x) < precision ||
            abs(res.hit_pos.x - mirror->pos.x + mirror->collider.radius.x) < precision;
        bool mirror_y = abs(res.hit_pos.y - mirror->pos.y - mirror->collider.radius.y) < precision ||
            abs(res.hit_pos.y - mirror->pos.y + mirror->collider.radius.y) < precision;

         if ( mirror_x ){
            mirrored_dir.x = -mirrored_dir.x;
        } else if ( mirror_y ){
            mirrored_dir.y = -mirrored_dir.y;
        }
        if(mirror_x || mirror_y){
            mirror->collider.transparency_type = TransparencyType_Opaque;

            mirror->last_raycast = 
                game_raycast(game, res.directly_hit_entity, res.hit_pos, mirrored_dir, mask, dbg); 

            //If a ray reaches the same mirror twice we declare that hit_found is false,
            //because it is likely to go in an infinite loop.
            if (mirror->last_raycast.final_hit_entity == mirror){
                res.hit_found = false;
                res.final_hit_entity = NULL;
            }else if (mirror->last_raycast.hit_found){
                res.final_hit_entity = mirror->last_raycast.final_hit_entity;
            }

            mirror->collider.transparency_type = TransparencyType_Mirror;
        }else{
            printf("Ray hit mirror, but we can't determine whether vertically or horizontally");
            res.hit_found = false;
        }

    }
    if(res.hit_found && res.directly_hit_entity->collider.transparency_type == TransparencyType_Camouflage){
        printf("Ray hits camouflage entity");
        Entity* entity = res.directly_hit_entity;
        entity->collider.transparency_type = TransparencyType_Transparent;
        entity->last_raycast = game_raycast(game, origin_entity, origin, dir, mask, dbg);
        entity->collider.transparency_type = TransparencyType_Camouflage;
        if(entity->last_raycast.hit_found && 
            entity->last_raycast.final_hit_entity->collider.camouflage_color == entity->collider.camouflage_color){
            
            printf("Camouflage worked.");
            res.final_hit_entity = entity->last_raycast.final_hit_entity;
        }else{
            printf("Camouflage failed: %d, %d\n", entity->last_raycast.final_hit_entity->type);
        }
    }


#ifdef DEBUG
    if (res.hit_found) {
        push_line(dbg, origin, res.hit_pos, v3(1, 0, 0));
    }
#endif
    return res;
}
