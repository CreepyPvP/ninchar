#ifndef GAME_H
#define GAME_H

#define FOR_POS_COLLIDER(game, block) \
{   \
    for (u32 i = 0; i < game->wall_count; ++i) {    \
        V3* pos = &game->wall[i].pos; \
        Collider* collider = &game->wall[i].collider; \
        block;  \
    }   \
    for (u32 i = 0; i < game->crate_count; ++i) {    \
        V3* pos = &game->crate[i].pos; \
        Collider* collider = &game->crate[i].collider; \
        block;  \
    }   \
    for (u32 i = 0; i < game->objective_count; ++i) {    \
        V3* pos = &game->objective[i].pos; \
        Collider* collider = &game->objective[i].collider; \
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
    ColliderType_Destroyable = 2,
};

struct Collider
{
    u32 type;
    V3 radius;
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

    Camera camera;
    u32 camera_state;

    Player player;
};


void game_load_assets();
void game_init(Game* game, Arena* arena, u32 stage);
void game_update(Game* game, RenderGroup* group, u8 inputs, float delta);

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
