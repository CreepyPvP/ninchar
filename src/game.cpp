#include "include/game.h"

#include "include/opengl_renderer.h"


TextureHandle ground_texture;
TextureHandle wall_texture;
TextureHandle crate_texture;

// TODO: move asset loading to asset queue. Then game has no opengl dependency
void game_load_assets()
{
    TextureLoadOp load_ground = texture_load_op(&ground_texture, "assets/ground.png");
    opengl_load_texture(&load_ground);
    free_texture_load_op(&load_ground);

    TextureLoadOp load_crate = texture_load_op(&crate_texture, "assets/crate.png");
    opengl_load_texture(&load_crate);
    free_texture_load_op(&load_crate);

    TextureLoadOp load_wall = texture_load_op(&wall_texture, "assets/wall.png");
    opengl_load_texture(&load_wall);
    free_texture_load_op(&load_wall);
};

void game_init(Game* game, Arena* arena)
{
    game->width = 10;
    game->height = 10;

    game->crate_count = 1;
    game->crate_cap = 1;
    game->crate = (Crate*) push_size(arena, sizeof(Crate) * game->crate_cap);
    game->crate[0].trans.pos = v3(4, 4, 1);

    game->wall_count = 1;
    game->wall_cap = 1;
    game->wall = (Wall*) push_size(arena, sizeof(Wall) * game->wall_cap);
    game->wall[0].trans.pos = v3(1, 1, 1);
}

void game_update(Game* game, RenderGroup* group)
{
    for (u32 y = 0; y < game->height; ++y) {
        for (u32 x = 0; x < game->width; ++x) {
            push_cube(group, v3(x, y, 0), v3(0.5), ground_texture, v3(1));
        }
    }

    for (u32 i = 0; i < game->crate_count; ++i) {
        push_cube(group, game->crate[i].trans.pos, v3(0.5), crate_texture, v3(1));
    }

    for (u32 i = 0; i < game->wall_count; ++i) {
        push_cube(group, game->wall[i].trans.pos, v3(0.5), wall_texture, v3(1));
    }
}
