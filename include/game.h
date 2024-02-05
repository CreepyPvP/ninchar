#ifndef GAME_H
#define GAME_H

#include <iostream>

#define FOR_POS_COLLIDER(game, block) \
{   \
    for (u32 i = 0; i < game->entity_type_count; i++){   \
        for(u32 j = 0; j < game->entity_types[i].count; j++){  \
            V3* pos = &game->entity_types[i].entity_list[j].pos;   \
            Entity* entity = &game->entity_types[i].entity_list[j];  \
            block;   \
        }   \
    }   \
}

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"
#include "include/camera.h"

#include <string>

enum CameraState
{
    CameraState_Locked = 0,
    CameraState_Free = 1,
};

struct Entity;
struct Game;
struct AABB;

class EntityType
{
public:
    std::string name;
    u32 count;
    u32 cap;

    Entity* entity_list;

    u32 extra_data_size;
    void* extra_data_list;

    TextureHandle* texture;
    V3 render_color;

    u32 load_tile_red;
    u32 load_tile_green;
    u32 load_tile_blue;


    void (*init)(Entity* entity, Game* game, u32 x, u32 y);
    void (*update)(Entity* entity, Game* game);
    void (*render)(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);

    void (*collision_response)(AABB a, AABB b, V2 dir, Game* game);
    V2 (*try_move_into)(AABB a, AABB b, V2 dir, Game* game);
};


struct Entity
{
    EntityType* type;
    V3 pos;
    V3 radius;
    void* extra_data;
};


struct ObjectiveExtraData
{
    bool broken;
};


struct Player
{
    V3 pos;
};

struct Game
{
    int entity_type_count;
    EntityType entity_types[10];

    u32 width;
    u32 height;

    Player player;

    Camera camera;
    u32 camera_state;

    int current_level;
    int total_level_count = 7;
    bool reset_stage;
};


void game_load_assets();
void game_init_entity_types(Game* game, TextureHandle* white_texture);
void game_init(Game* game, Arena* arena, u32 stage);
void game_update(Game* game, u8 inputs, float delta);
void game_render(Game* game, RenderGroup* group, RenderGroup* dbg);

void game_reset_camera(Game* game);
void game_toggle_camera_state(Game* game);

void add_entity_type(EntityType* type, Game* game);

struct AABB
{
    V3* pos;
    Entity* entity;
};

inline AABB aabb(V3* pos, Entity* entity) 
{
    return { pos, entity };
}

void move_and_collide(AABB aabb, V2 dir, Game* game);

#endif
