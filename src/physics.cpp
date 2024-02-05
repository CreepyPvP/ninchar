#include "include/game.h"

#include "include/types.h"
#include "include/game_math.h"

#include <iostream>






V3 far_away = v3(1000000, 1000000, 1000000);
float box_gap = 0.00001f;

V2 get_collided_movement(AABB a, V2 dir, Game* game);
V2 distance_towards(AABB a, AABB b, V2 dir);

float clamp_abs(float a, float b){
    if (is_zero(b)){
        return 0;
    } else if (b > 0){
        return min(a, b);
    } else{
        return max(a, b);
    }
}


bool intersects(AABB a, AABB b)
{
    return a.pos->x - a.entity->radius.x < b.pos->x + b.entity->radius.x &&
           a.pos->x + a.entity->radius.x > b.pos->x - b.entity->radius.x &&
           a.pos->y - a.entity->radius.y < b.pos->y + b.entity->radius.y &&
           a.pos->y + a.entity->radius.y > b.pos->y - b.entity->radius.y &&
           a.pos->z - a.entity->radius.z < b.pos->z + b.entity->radius.z &&
           a.pos->z + a.entity->radius.z > b.pos->z - b.entity->radius.z;
}

V2 expand_slightly(V2 dir){
    V2 res;
    if (is_zero(dir.x)){
        res.x = dir.x;
    } else if (dir.x > 0) {
        res.x = dir.x + box_gap;
    } else {
        res.x = dir.x - box_gap;
    }
    if (is_zero(dir.y)){
        res.y = dir.y;
    } else if (dir.y > 0) {
        res.y = dir.y + box_gap;
    } else {
        res.y = dir.y - box_gap;
    }
    return res;
}

void do_collision_response(AABB a, AABB b, V2 dir, Game* game) 
{
    if (b.entity->type->collider_type == ColliderType_Moveable) {
        V2 to = distance_towards(a, b, dir);
        move_and_collide(b, v2(clamp_abs(dir.x, dir.x - to.x), clamp_abs(dir.y, dir.y - to.y)), game);
    } else if (b.entity->type->collider_type == ColliderType_Objective) {
        ObjectiveExtraData* data = (ObjectiveExtraData*)b.entity->extra_data;
        data->broken = true;
    }
}

void move_and_push_boxes(AABB a, V2 dir, Game* game)
{
    V3 new_pos = v3(a.pos->x + dir.x, a.pos->y + dir.y, a.pos->z);
    V3 old_pos = *a.pos;

    AABB old_aabb = aabb(&old_pos, a.entity);
    AABB new_aabb = aabb(&new_pos, a.entity);
    *a.pos = far_away;

    FOR_POS_COLLIDER(game, {
        AABB b = aabb(pos, entity);
        if (intersects(new_aabb, b)) {
            do_collision_response(old_aabb, b, expand_slightly(dir), game);
        }
    });

    *a.pos = new_pos;
}

V2 distance_towards(AABB a, AABB b, V2 dir) 
{
    V2 res;
    if (dir.x >= 0) {
        res.x = b.pos->x - a.pos->x - a.entity->radius.x - b.entity->radius.x;
    } else {
        res.x = b.pos->x - a.pos->x + a.entity->radius.x + b.entity->radius.x;
    }

    if (dir.y >= 0) {
        res.y = b.pos->y - a.pos->y - a.entity->radius.y - b.entity->radius.y;
    } else {
        res.y = b.pos->y - a.pos->y + a.entity->radius.y + b.entity->radius.y;
    }

    return res;
}

V2 try_move_into(AABB a, AABB b, V2 dir, Game* game) 
{
    switch (b.entity->type->collider_type) {
        case ColliderType_Static: {
            return distance_towards(a, b, dir);
        } break;

        case ColliderType_Moveable: {
            V2 to = distance_towards(a, b, dir);
            V2 tmp = get_collided_movement(b, v2(clamp_abs(dir.x, dir.x - to.x), clamp_abs(dir.y, dir.y - to.y)), game);
            return v2(to.x + tmp.x, to.y + tmp.y);
        } break;

        case ColliderType_Objective: {
            return dir;
        }
    }

    return v2(0);
}

V2 get_collided_movement(AABB a, V2 dir, Game* game)
{
    V3 new_pos = v3(a.pos->x + dir.x, a.pos->y + dir.y, a.pos->z);
    V3 old_pos = *a.pos;
    *a.pos = far_away;

    V2 res = dir;
    
    AABB new_aabb = aabb(&new_pos, a.entity);
    AABB old_aabb = aabb(&old_pos, a.entity);

    FOR_POS_COLLIDER(game, {
        AABB b = aabb(pos, entity);
        if (intersects(new_aabb, b)) {
            V2 move_into = try_move_into(old_aabb, b, expand_slightly(dir), game);
            res.x = clamp_abs(res.x, move_into.x);
            res.y = clamp_abs(res.y, move_into.y);
        }
    });

    *a.pos = old_pos;
    return res;
}

void move_and_collide(AABB a, V2 dir, Game* game)
{
    V2 actual_movement = get_collided_movement(a, dir, game);
    if (!is_zero(actual_movement.x) || !is_zero(actual_movement.y)) {
        move_and_push_boxes(a, actual_movement, game);
    }
}
