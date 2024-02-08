#ifndef GAME_H
#define GAME_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"
#include "include/camera.h"

#define ENTITY_CAP 1000
#define ACCESS_ENITTY_CAP 10

enum CameraState
{
    CameraState_Locked,
    CameraState_Free,
};

enum ColliderType
{
    ColliderType_None,
    ColliderType_Static,
    ColliderType_Moveable,
};

enum EntityType
{
    EntityType_Player,
    EntityType_Crate,
    EntityType_Wall,
    EntityType_Objective,
    EntityType_Enemy,

    EntityType_Count,
};


struct EntityRef
{
    u32 id;
    // TODO: Add generation here
};

struct Collider
{
    ColliderType type;
    V3 radius;
};

struct Objective
{
    bool broken;
};

struct Entity 
{
    EntityType type;

    V3 pos;
    Collider collider;
    float rotation;

    V3 color;
    TextureHandle texture;

    union {
        Objective objective;
    };
};

struct EntityList
{
    u32 entity_count;
    EntityRef entity_refs[ACCESS_ENITTY_CAP];
};

struct Game
{
    u32 width;
    u32 height;

    Entity* entities;
    u32 entity_count;

    EntityRef player;
    EntityList enemies;
    EntityList objectives;

    Camera camera;
    u32 camera_state;

    int current_level;
    int total_level_count = 7;
    bool reset_stage;
};


void game_load_assets();
void game_init(Game* game, Arena* arena, u32 stage, TextureHandle white);
void game_update(Game* game, u8 inputs, float delta, RenderGroup* group, RenderGroup* dbg);
void game_render(Game* game, RenderGroup* group, RenderGroup* dbg);

bool game_raycast(Game* game, V3 origin, V3 dir, EntityRef* ref, RenderGroup* dbg);

void game_reset_camera(Game* game);
void game_toggle_camera_state(Game* game);

EntityRef push_entity(Entity entity, Game* game);
Entity* get_entity(EntityRef ref, Game* game);
void push_entity_to_list(EntityList* list, EntityRef ref);

void move_and_collide(Entity* entity, V2 dir, Game* game);

#endif
