#include "include/game.h"

#include "include/types.h"
#include "include/game_math.h"


V3 far_away = v3(1000, 1000, 1000);

float clamp_abs(float a, float b){
    if (b < 0.0001 && b > -0.0001){
        return 0;
    } else if (b > 0){
        return min(a, b);
    } else{
        return max(a, b);
    }
}

bool intersects(V3 pa, Collider* ca, V3 pb, Collider* cb)
{
    return pa.x - ca->radius.x < pb.x + cb->radius.x &&
           pa.x + ca->radius.x > pb.x - cb->radius.x &&
           pa.y - ca->radius.y < pb.y + cb->radius.y &&
           pa.y + ca->radius.y > pb.y - cb->radius.y &&
           pa.z - ca->radius.z < pb.z + cb->radius.z &&
           pa.z + ca->radius.z > pb.z - cb->radius.z;
}

void move_and_push_boxes(V3* pos, V2 dir, Game* game)
{
    V3 new_pos = v3(pos->x + dir.x, pos->y + dir.y, pos->z);
    *pos = far_away;

    // TODO: handle collisions here

    *pos = new_pos;
}

V2 distance_towards(V3 pos, Collider* col, V3 other_pos, Collider* other, V2 dir) 
{
    V2 res;
    if (dir.x > 0) {
        res.x = other_pos.x - pos.x - col->radius.x - other->radius.x;
    } else {
        res.x = other_pos.x - pos.x + col->radius.x + other->radius.x;
    }

    if (dir.y > 0) {
        res.y = other_pos.y - pos.y - col->radius.y - other->radius.y;
    } else {
        res.y = other_pos.y - pos.y + col->radius.y + other->radius.y;
    }

    return res;
}

V2 try_move_into(V3 pos, Collider* col, V3 other_pos, Collider* other, V2 dir, Game* game) 
{
    switch (other->type) {
        case ColliderType_Static: {
            return distance_towards(pos, col, other_pos, other, dir);
        } break;

        case ColliderType_Moveable: {
            return distance_towards(pos, col, other_pos, other, dir);
        } break;

        case ColliderType_Destroyable: {
            // return dir;
        }
    }

    return v2(0);
}

V2 get_collided_movement(V3* pos, Collider* col, V2 dir, Game* game)
{
    V3 new_pos = v3(pos->x + dir.x, pos->y + dir.y, pos->z);
    V3 old_pos = *pos;
    *pos = far_away;

    V2 res = dir;

    // TODO Clean this up
    for (u32 i = 0; i < game->wall_count; ++i) {
        V3 other_pos = game->wall[i].pos;
        Collider* other = &game->wall[i].collider;
        if (intersects(new_pos, col, other_pos, other)) {
            V2 move_into = try_move_into(old_pos, col, other_pos, other, dir, game);
            res.x = clamp_abs(res.x, move_into.x);
            res.y = clamp_abs(res.y, move_into.y);
        }
    }
    for (u32 i = 0; i < game->crate_count; ++i) {
        V3 other_pos = game->crate[i].pos;
        Collider* other = &game->crate[i].collider;
        if (intersects(new_pos, col, other_pos, other)) {
            V2 move_into = try_move_into(old_pos, col, other_pos, other, dir, game);
            res.x = clamp_abs(res.x, move_into.x);
            res.y = clamp_abs(res.y, move_into.y);
        }
    }

    *pos = old_pos;
    return res;
}

void move_and_collide(V3* pos, Collider* col, V2 dir, Game* game)
{
    V2 actual_movement = get_collided_movement(pos, col, dir, game);
    if (actual_movement.x != 0 || actual_movement.y != 0) {
        move_and_push_boxes(pos, actual_movement, game);
    }
}
