#include "include/game.h"

#include "include/types.h"
#include "include/game_math.h"


V2int far_away = {-1000000, -1000000};
float box_gap = 0.00001f;

V2int get_collided_movement(Entity* a, V2int dir, Game* game);
V2int distance_towards(Entity* a, Entity* b, V2int dir);



void Collider::set_radius(int new_int_radius, float z){
        int_radius = {new_int_radius, new_int_radius};
        float_radius = v2int_to_v3float(int_radius, z);
}


int clamp_abs(int a, int b){
    if (b == 0){
        return 0;
    } else if (b > 0){
        return min(a, b);
    } else{
        return int_max(a, b);
    }
}


bool intersects(Entity* a, Entity* b)
{
    return a->int_pos.x - a->collider.int_radius.x < b->int_pos.x + b->collider.int_radius.x &&
           a->int_pos.x + a->collider.int_radius.x > b->int_pos.x - b->collider.int_radius.x &&
           a->int_pos.y - a->collider.int_radius.y < b->int_pos.y + b->collider.int_radius.y &&
           a->int_pos.y + a->collider.int_radius.y > b->int_pos.y - b->collider.int_radius.y;
}

void do_collision_response(Entity* a, Entity* b, V2int dir, Game* game) 
{
    if (b->collider.type == ColliderType_Moveable) {
        V2int to = distance_towards(a, b, dir);
        move_and_collide(b, {clamp_abs(dir.x, dir.x - to.x), clamp_abs(dir.y, dir.y - to.y)}, game);
    } else if (b->type == EntityType_Objective) {
        b->objective.broken = true;
        b->collider.transparency_type = TransparencyType_Transparent;
    }
}

void move_and_push_boxes(Entity* a, V2int dir, Game* game)
{
    V2int new_pos = {a->int_pos.x + dir.x, a->int_pos.y + dir.y};
    V2int old_pos = a->int_pos;

    a->int_pos = far_away;

    Entity tmp = *a;
    for (u32 i = 0; i < game->entity_count; ++i) {
        tmp.int_pos = new_pos;
        Entity* b = game->entities + i;
        if (intersects(&tmp, b)) {
            tmp.int_pos = old_pos;
            do_collision_response(&tmp, b, dir, game);
        }
    }

    a->int_pos = new_pos;
    a->pos = v2int_to_v3float(a->int_pos, a->pos.z);
}

V2int distance_towards(Entity* a, Entity* b, V2int dir) 
{
    V2int res;
    if (dir.x >= 0) {
        res.x = b->int_pos.x - a->int_pos.x - a->collider.int_radius.x - b->collider.int_radius.x;
    } else {
        res.x = b->int_pos.x - a->int_pos.x + a->collider.int_radius.x + b->collider.int_radius.x;
    }

    if (dir.y >= 0) {
        res.y = b->int_pos.y - a->int_pos.y - a->collider.int_radius.y - b->collider.int_radius.y;
    } else {
        res.y = b->int_pos.y - a->int_pos.y + a->collider.int_radius.y + b->collider.int_radius.y;
    }

    return res;
}

V2int try_move_into(Entity* a, Entity* b, V2int dir, Game* game) 
{
    switch (b->collider.type) {
        case ColliderType_Static: {
            return distance_towards(a, b, dir);
        } break;

        case ColliderType_Moveable: {
            V2int to = distance_towards(a, b, dir);
            V2int tmp = get_collided_movement(b, {clamp_abs(dir.x, dir.x - to.x), clamp_abs(dir.y, dir.y - to.y)}, game);
            return {to.x + tmp.x, to.y + tmp.y};
        } break;
    }

    return dir;
}

V2int get_collided_movement(Entity* a, V2int dir, Game* game)
{
    V2int new_pos = {a->int_pos.x + dir.x, a->int_pos.y + dir.y};
    V2int old_pos = a->int_pos;
    a->int_pos = far_away;

    V2int res = dir;

    Entity tmp = *a;
    for (u32 i = 0; i < game->entity_count; ++i) {
        Entity* b = game->entities + i;
        tmp.int_pos = new_pos;
        if (intersects(&tmp, b)) {
            tmp.int_pos = old_pos;
            V2int move_into = try_move_into(&tmp, b, dir, game);
            res.x = clamp_abs(res.x, move_into.x);
            res.y = clamp_abs(res.y, move_into.y);
        }
    }

    a->int_pos = old_pos;
    return res;
}

void move_and_collide(Entity* a, V2int dir, Game* game)
{
    V2int actual_movement = get_collided_movement(a, dir, game);
    if (!is_zero(actual_movement.x) || !is_zero(actual_movement.y)) {
        move_and_push_boxes(a, actual_movement, game);
    }
}
