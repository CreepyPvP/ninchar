#include "include/game.h"


void game_init(Game* game, Arena* arena)
{
    game->width = 10;
    game->height = 10;
    game->grid = (u8*) push_size(arena, sizeof(u8) * game->width * game->height);

    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            game->grid[x + y * game->width] = 1;
        }
    }
}

void game_update(Game* game, RenderGroup* group, TextureHandle texture)
{
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            u8 type = game->grid[x + y * game->width];

            if (type) {
                push_cube(group, v3(x, y, 0), v3(0.5), texture, v3(0.8));
            }
        }
    }
}
