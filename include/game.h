#ifndef GAME_H
#define GAME_H

#include "include/types.h"
#include "include/arena.h"
#include "include/renderer.h"
#include "include/camera.h"

#define ENTITY_CAP 1000
#define ACCESS_ENITTY_CAP 100

#define ENEMY_VISION    (1 << EntityType_Player) | (1 << EntityType_Wall) |     \
                        (1 << EntityType_Crate) | (1 << EntityType_Objective) |  \
                        (1 << EntityType_Enemy) | (1 << EntityType_MirrorWall) | \
                        (1 << EntityType_ColoredWall)

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

enum TransparencyType
{
    TransparencyType_Opaque,
    TransparencyType_Transparent,
    TransparencyType_Mirror,
    TransparencyType_Camouflage,
};

struct Entity;

struct RaycastResult
{
    bool hit_found;
    float t;
    V3 origin;
    V3 hit_pos;
    Entity* directly_hit_entity;
    Entity* final_hit_entity;
};

enum EntityType
{
    EntityType_Player,
    EntityType_Crate,
    EntityType_Wall,
    EntityType_GlassWall,
    EntityType_Objective,
    EntityType_Enemy,
    EntityType_MirrorWall,
    EntityType_ColoredWall,

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
    TransparencyType transparency_type;
    V3 radius;

    // The player is invisible to enemies if behind the player 
    // is a wall of the same camouflage color as the player.
    u32 camouflage_color;
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
    bool transparent;

    // If a ray hits a mirror, the mirror stores the ray that bounces away from the mirror.
    RaycastResult last_raycast;

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

    bool next_stage;
    bool reset_stage;
};


void game_load_assets();
void game_init(Game* game, Arena* arena, u32 stage, TextureHandle white);
void game_update(Game* game, u8 inputs, float delta, RenderGroup* group, RenderGroup* dbg);
void game_render(Game* game, RenderGroup* group, RenderGroup* transparent, RenderGroup* dbg);

RaycastResult game_raycast(Game* game, Entity* origin_entity, V3 origin, V3 dir, u32 mask, RenderGroup* dbg);

void game_reset_camera(Game* game);
void game_toggle_camera_state(Game* game);

EntityRef push_entity(Entity entity, Game* game);
Entity* get_entity(EntityRef ref, Game* game);
void push_entity_to_list(EntityList* list, EntityRef ref);

void move_and_collide(Entity* entity, V2 dir, Game* game);

#endif
