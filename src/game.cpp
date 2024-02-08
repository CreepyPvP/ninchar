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

            if (curr[0] == 0 && curr[1] == 0 && curr[2] == 0) {
                entity.type = EntityType_Wall;
                entity.collider.type = ColliderType_Static;
                entity.texture = wall_texture;
                push_entity(entity, game);
            }

            if (curr[0] == 50 && curr[1] == 50 && curr[2] == 5) {
                entity.type = EntityType_GlassWall;
                entity.collider.type = ColliderType_Static;
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
                entity.collider.radius = v3(0.35, 0.35, 0.7);
                entity.color = v3(0, 0, 1);
                game->player = push_entity(entity, game);
            }

            if (curr[0] == 1 && curr[1] == 125) {
                entity.type = EntityType_Objective;
                entity.color = v3(0, 1, 0);
                push_entity(entity, game);
            }

            if (curr[0] == 0 && curr[1] == 255 && curr[2] == 0) {
                entity.type = EntityType_Enemy;
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

        push_spotlight(group, enemy->pos, facing, fov);

#ifdef DEBUG
        game_raycast(game, enemy->pos, facing, ENEMY_VISION, NULL, dbg);
        game_raycast(game, enemy->pos, left, ENEMY_VISION, NULL, dbg);
        game_raycast(game, enemy->pos, right, ENEMY_VISION, NULL, dbg);
#endif

        Entity* player = get_entity(game->player, game);
        V3 player_dir = v3(player->pos.x - enemy->pos.x, 
                           player->pos.y - enemy->pos.y, 
                           player->pos.z - enemy->pos.z);

        EntityRef ray_res;
        if (game_raycast(game, enemy->pos, player_dir, ENEMY_VISION, &ray_res, dbg)) {
            if (ray_res.id == game->player.id) {
                if (dot(norm(player_dir), facing) > dot(norm(left), facing)) {
                    game->reset_stage = true;
                }
            }
        }

        enemy->rotation += 0.4 * delta;
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

bool game_raycast(Game* game, V3 origin, V3 dir, u32 mask, EntityRef* ref, RenderGroup* dbg)
{
    float t;
    bool hit_found = false;
    V3 hit;
    
    for (u32 i = 0; i < game->entity_count; ++i) {
        Entity* entity = game->entities + i;

        if (!(1 & (mask >> entity->type))) {
            continue;
        }

        V3 chit;
        float ct;
        if (hit_bounding_box(origin, dir, entity->pos, entity->collider.radius, &chit, &ct)) {
            if (!hit_found || ct < t) {
                hit_found = true;
                t = ct;
                hit = chit;
                if (ref) {
                    ref->id = i;
                }
            }
        }
    }

#ifdef DEBUG
    if (hit_found) {
        push_line(dbg, origin, hit, v3(1, 0, 0));
    }
#endif
    return hit_found;
}
