#include "include/entity.h"

#include "include/game_math.h"
#include "include/util.h"



u32 get_entity_type_index(EntityTypeId id, Game* game){
    if(game->types[id].id == id){
        return id;
    }
    for (u32 i=0; i<game->type_count; i++){
        if ( game->types[i].id == id){
            return i;
        }
    }
    return -1;
}

//Assumes entity is a ColliderEntity
void collider_entity_standard_init(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b)
{
        entity->pos = v3(x, y, 1);
        ((ColliderEntity*)entity)->radius = v3(0.5);
        ((ColliderEntity*)entity)->transparent = false;
}





void player_init(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b){
    entity->pos = v3(x, y, 1);
    ((PlayerEntity*)entity)->radius = v3(0.35, 0.35, 0.7);
    ((PlayerEntity*)entity)->transparent = false;
    game->player = (PlayerEntity*) entity;
}

void player_update(Entity* entity, Game* game, u8 inputs, float delta){
    if (game->camera_state == CameraState_Free) {
        update_camera(&game->camera, inputs, delta);
    } else {

        V2 movement = v2(0);

        // w
        if (inputs & (1 << 0)) {
            movement.y += 1;
        }
        // s
        if (inputs & (1 << 1)) {
            movement.y -= 1;
        }
        // a
        if (inputs & (1 << 2)) {
            movement.x -= 1;
        }
        // d
        if (inputs & (1 << 3)) {
            movement.x += 1;
        }

        movement = norm(movement);

        movement.x *= delta * 10;
        movement.y *= delta * 10;

        move_and_collide(aabb(&entity->pos, (PlayerEntity*)entity), v2(movement.x, 0), game);
        move_and_collide(aabb(&entity->pos, (PlayerEntity*)entity), v2(0, movement.y), game);
    }
}

void player_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    push_cube(group, entity->pos, v3(0.35, 0.35, 0.7), group->commands->white, v3(0, 0, 1));
}




void enemy_init(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b)
{
    collider_entity_standard_init(entity, game, x,y,r,g,b);
    ((EnemyEntity*)entity)->rotation = 0;
    ((EnemyEntity*)entity)->rotation_speed = 1.6f;
}


#define ENEMY_RAY_COUNT 80

void enemy_update(Entity* entity, Game* game, u8 inputs, float delta)
{
    EnemyEntity* enemy = (EnemyEntity*)entity;
    enemy->rotation -= (enemy->rotation_speed * delta);

    PlayerEntity* player = game->player;

    V3 enemy_to_player = v3(player->pos.x - enemy->pos.x, player->pos.y - enemy->pos.y, 0);

    V3 normed_etp = norm(enemy_to_player);
    float squared_distance = (enemy_to_player.x*enemy_to_player.x) + (enemy_to_player.y* enemy_to_player.y);

    RaycastResult etp_ray = game_raycast(game, enemy->pos, normed_etp);

    if(etp_ray.hit_entity->type->id == EntityType_Player){
        const V2 facing = v2(-sin(enemy->rotation),
                     cos(enemy->rotation));
        const V2 side = v2(-facing.y, facing.x);
        const float fov = 0.3;

        
        const V2 r = norm(v2((1 - fov) * facing.x - fov * side.x,
                (1 - fov) * facing.y - fov * side.y));

        if ((normed_etp.x *facing.x) + (normed_etp.y * facing.y) >= 
            (r.x * facing.x) + (r.y * facing.y) ){
            game->reset_stage = true;
        }
    }
    

}


void enemy_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    push_cube(group, entity->pos, v3(0.5), *(TextureHandle*)(entity->type->render_data), v3(0.5, 0, 0));
    //TODO: Render field of view.
    float rotation = ((EnemyEntity*)entity)->rotation;


    const V3 pos = entity->pos;
    const V2 facing = v2(-sin(rotation),
                         cos(rotation));
    const V2 side = v2(-facing.y, facing.x);
    const float fov = 0.3;

    float o = 2.0f / ENEMY_RAY_COUNT - 1;
    for (u32 i = 0; i < ENEMY_RAY_COUNT - 1; ++i) {
        o += 2.0f / ENEMY_RAY_COUNT;

        V3 r = v3((1 - fov) * facing.x + o * fov * side.x,
                  (1 - fov) * facing.y + o * fov * side.y, 0);
        game_render_raycast(game, pos, r, dbg);

    }



}





void entity_standard_update(Entity* entity, Game* game, u8 inputs, float delta) {}


void entity_glass_wall_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg){
    push_cube(group, entity->pos, v3(0.5), group->commands->white, v3(0.5,1,1));
}

//Assumes render_data is a TextureHandle
void entity_texture_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    push_cube(group, entity->pos, v3(0.5),  *(TextureHandle*)(entity->type->render_data), v3(1));
}

//Assumes render_data is a ColorHandle
void entity_color_cube_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg){
    push_cube(group, entity->pos, v3(0.5), group->commands->white, ((ColorHandle*)entity->type->render_data)->color);
}





//Assumes entity is an ObjectiveEntity
void objective_init(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b){
    collider_entity_standard_init(entity, game, x, y, r, g, b);
    ((ObjectiveEntity*)entity)->broken = false;
}

//Assumes entity is an ObjectiveEntity
void objective_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    if(!((ObjectiveEntity*)entity)->broken){
        entity_color_cube_render(entity, game, group, dbg);
    }
}

