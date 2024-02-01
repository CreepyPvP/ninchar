#ifndef GAME_H
#define GAME_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"

struct Game
{
    u32 width;
    u32 height;
    u8* grid;
};

void game_init(Game* game, Arena* arena);
void game_update(Game* game, RenderGroup* group);

#endif
