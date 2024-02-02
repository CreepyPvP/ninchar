#ifndef GAME_H
#define GAME_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"
#include "include/camera.h"

enum CameraState
{
    CameraState_Locked = 0,
    CameraState_Free = 1,
};

struct Collider
{
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

    Camera camera;
    u32 camera_state;

    Player player;
};


void game_load_assets();
void game_init(Game* game, Arena* arena, u32 stage);
void game_update(Game* game, RenderGroup* group, u8 inputs, float delta);

void game_toggle_camera_state(Game* game);


void move_and_collide(V3* pos, Collider* col, V2 dir, Game* game);

#endif
