#ifndef GAME_H
#define GAME_H

#include <iostream>

#define FOR_POS_COLLIDER(game, block) \
{   \
    for (u32 i = 0; i < game->type_count; i++){   \
        for(u32 j = 0; j < game->types[i].count; j++){  \
            V3* pos = &game->types[i].entity_list[j].pos;   \
            Entity* entity = &game->types[i].entity_list[j];  \
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



class EntityType;

struct Entity
{
    EntityType* type;
    V3 pos;
};

struct ColliderEntity : Entity
{
    V3 radius;
};

struct ObjectiveEntity : ColliderEntity
{
    bool broken;
};


struct Player
{
    V3 pos;
};



enum EntityTypeId
{
    EntityType_Wall = 0,
    EntityType_Crate = 1,
    EntityType_Objective = 2,
};







struct Game;
struct AABB;

class EntityType
{
private:
    //'entity_list' list will usually not store Entity struct instances, but instances of a struct that inherits from Entity.
    Entity* entity_list;
public:
    //'sizeof_entity' should specify how large the struct is whose instances get stored in 'entity_list'.
    u32 sizeof_entity;

    //Entries of the 'entity_list' should be accessed using this get function. 'entity_list' has been made private to prevent other accesses.
    Entity* get_entity(int index){
        return (Entity*)(((char*)entity_list) + (sizeof_entity * index));
    }

    void set_entity_list(Entity* new_entity_list){
        entity_list = new_entity_list;
    }

    EntityTypeId id;
    u32 count;
    u32 cap;

    //Render data
    RenderData* render_data;

    //Data for loading entities of this type from png
    u32 load_tile_red;
    u32 load_tile_green;
    u32 load_tile_blue;

    //Every entity type need to implement its own init, update and render function.
    void (*init)(Entity* entity, Game* game, u32 x, u32 y);
    void (*update)(Entity* entity, Game* game);
    void (*render)(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);

    //The physics system ignores this entity type if collideable is false.
    bool collideable;
    //Every collideable entity type needs to implement a collision_response and try_move_into function.
    void (*collision_response)(AABB a, AABB b, V2 dir, Game* game);
    V2 (*try_move_into)(AABB a, AABB b, V2 dir, Game* game);

};

struct Game
{
    int type_count;
    EntityType types[10];

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
void game_init_entity_types(Game* game);
void game_init(Game* game, Arena* arena, u32 stage);
void game_update(Game* game, u8 inputs, float delta);
void game_render(Game* game, RenderGroup* group, RenderGroup* dbg);

void game_reset_camera(Game* game);
void game_toggle_camera_state(Game* game);

void add_entity_type(EntityType* type, Game* game);

struct AABB
{
    V3* pos;
    ColliderEntity* entity;
};

inline AABB aabb(V3* pos, ColliderEntity* entity) 
{
    return { pos, entity };
}

void move_and_collide(AABB aabb, V2 dir, Game* game);

#endif
