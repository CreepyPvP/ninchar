#ifndef GAME_H
#define GAME_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"


struct Transform
{
    V3 pos;
};

struct Crate
{
    Transform trans;
};

struct Wall
{
    Transform trans;
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
};


void game_load_assets();
void game_init(Game* game, Arena* arena);
void game_update(Game* game, RenderGroup* group);

#endif
