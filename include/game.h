#ifndef GAME_H
#define GAME_H

#include <iostream>

#define FOR_POS_COLLIDER(game, block) \
{   \
    for (u32 i = 0; i < game->type_count; i++){ \
        if (game->types[i].collideable){  \
            for (u32 j = 0; j < game->types[i].count; j++){  \
                V3* pos = &game->types[i].get_entity(j)->pos;   \
                ColliderEntity* entity = (ColliderEntity*)(game->types[i].get_entity(j));  \
                block;   \
            }   \
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
    bool transparent;
};

struct ObjectiveEntity : ColliderEntity
{
    bool broken;
};


struct PlayerEntity : ColliderEntity {};

struct EnemyEntity : ColliderEntity
{
    float rotation;
    float rotation_speed;
};

struct RaycastResult{
    float t;
    bool hit_found;
    V3 hit;
    ColliderEntity* hit_entity;
};



enum EntityTypeId
{
    EntityType_Wall = 0,
    EntityType_Crate = 1,
    EntityType_Objective = 2,
    EntityType_Player = 3,
    EntityType_Enemy = 4,
    EntityType_GlassWall = 5,
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

    //Render data can be a TextureHandle, a ColorHandle a ModelHandle or something else that inherits from the RenderData struct.
    RenderData* render_data;

    //Data for loading entities of this type from png
    //If one of the colors is -1, then that particular color gets ignored for loading.
    u32 load_tile_red;
    u32 load_tile_green;
    u32 load_tile_blue;

    //Every entity type need to implement its own init, update and render function.
    void (*init)(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b);
    void (*update)(Entity* entity, Game* game, u8 inputs, float delta);
    void (*render)(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);

    //The physics system ignores this entity type if collideable is false.
    bool collideable;
    //Every collideable entity type needs to implement a collision_response and try_move_into function.
    void (*collision_response)(AABB a, AABB b, V2 dir, Game* game);
    V2 (*try_move_into)(AABB a, AABB b, V2 dir, Game* game);

    //transparent entities are ignored by raycasting.
    bool always_transparent;

};

struct Game
{
    int type_count;
    EntityType types[10];

    u32 width;
    u32 height;

    PlayerEntity* player;

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
void game_render_raycast(Game* game, V3 origin, V3 dir, RenderGroup* dbg);

RaycastResult game_raycast(Game* game, V3 origin, V3 dir);

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
