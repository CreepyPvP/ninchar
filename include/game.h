#ifndef GAME_H
#define GAME_H

#include <iostream>

#define FOR_POS_COLLIDER(game, block) \
{   \
    V3* pos;    \
    Collider* collider;  \
    EntityRef entity_ref;  \
    entity_ref.type = EntityType_Wall; \
    for (u32 i = 0; i < game->wall_count; ++i) {    \
        pos = &game->wall[i].pos; \
        collider = &game->wall[i].collider; \
        entity_ref.id = i; \
        block;  \
    }   \
    entity_ref.type = EntityType_Crate; \
    for (u32 i = 0; i < game->crate_count; ++i) {    \
        pos = &game->crate[i].pos; \
        collider = &game->crate[i].collider; \
        entity_ref.id = i; \
        block;  \
    }   \
    entity_ref.type = EntityType_Objective; \
    for (u32 i = 0; i < game->objective_count; ++i) {    \
        pos = &game->objective[i].pos; \
        collider = &game->objective[i].collider; \
        entity_ref.id = i; \
        block;  \
    }   \
}

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"
#include "include/camera.h"

enum CameraState
{
    CameraState_Locked = 0,
    CameraState_Free = 1,
};

enum ColliderType
{
    ColliderType_Static = 0,
    ColliderType_Moveable = 1,
    ColliderType_Objective = 2,
};

enum EntityType
{
    EntityType_Player = 0,
    EntityType_Crate = 1,
    EntityType_Wall = 2,
    EntityType_Objective = 3,
    EntityType_Enemy = 4,
};


struct EntityRef
{
    u32 type;
    u32 id;
    // TODO: Add generation here
};

struct Collider
{
    u32 type;
    V3 radius;
    void* extra_data;
};

struct Crate
{
    V3 pos;
    Collider collider;
};

struct Wall
{
    V3 pos;
    Collider collider;
};

struct Objective
{
    V3 pos;
    Collider collider;
    bool broken;

    static void on_collide(Objective* this_objective) {
        this_objective->broken = true;
    }
};

struct Enemy
{
    V3 pos;
    float rotation;
};

struct Player
{
    V3 pos;
};

struct Game
{
    u32 width;
    u32 height;

    Wall* wall;
    u32 wall_count;
    u32 wall_cap;

    Crate* crate;
    u32 crate_count;
    u32 crate_cap;

    Objective* objective;
    u32 objective_count;
    u32 objective_cap;

    Enemy* enemy;
    u32 enemy_count;
    u32 enemy_cap;

    Player player;

    Camera camera;
    u32 camera_state;

    int current_level;
    int total_level_count = 7;
    bool reset_stage;
};


void game_load_assets();
void game_init(Game* game, Arena* arena, u32 stage);
void game_update(Game* game, u8 inputs, float delta, RenderGroup* dbg);
void game_render(Game* game, RenderGroup* group, RenderGroup* dbg);

bool game_raycast(Game* game, V3 origin, V3 dir, EntityRef* ref, RenderGroup* dbg);

void game_reset_camera(Game* game);
void game_toggle_camera_state(Game* game);


struct AABB
{
    V3* pos;
    Collider* collider;
};

inline AABB aabb(V3* pos, Collider* collider) 
{
    return { pos, collider };
}

void move_and_collide(AABB aabb, V2 dir, Game* game);

#endif
