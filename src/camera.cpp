#include "include/camera.h"

#include <math.h>

#include "include/game_math.h"

#define CAMERA_SPEED 5

void init_camera(Camera* camera, V3 pos, V3 front)
{
    camera->pos = pos;
    camera->front = norm(front);
    camera->yaw = 0;
    camera->pitch = 0;

    camera->locked = true;
}

void update_camera(Camera* camera, u8 buttons_pressed, float delta)
{
    if (camera->locked) {
        return;
    }

    V2 movement = v2(0);

    // w
    if (buttons_pressed & (1 << 0)) {
        movement.x += 1;
    }
    // s
    if (buttons_pressed & (1 << 1)) {
        movement.x -= 1;
    }
    // a
    if (buttons_pressed & (1 << 2)) {
        movement.y -= 1;
    }
    // d
    if (buttons_pressed & (1 << 3)) {
        movement.y += 1;
    }

    float distance = sqrt(movement.x * movement.x + movement.y * movement.y);
    if (distance < 0.1) {
        return;
    }
    movement = v2(movement.x / distance, movement.y / distance);

    V3 right = norm(cross(camera->front, v3(0, 0, 1)));

    camera->pos.x += camera->front.x * movement.x * delta * CAMERA_SPEED;
    camera->pos.y += camera->front.y * movement.x * delta * CAMERA_SPEED;
    camera->pos.z += camera->front.z * movement.x * delta * CAMERA_SPEED;
    camera->pos.x += right.x * movement.y * delta * CAMERA_SPEED;
    camera->pos.y += right.y * movement.y * delta * CAMERA_SPEED;
    camera->pos.z += right.z * movement.y * delta * CAMERA_SPEED;
};

void update_camera_mouse(Camera* camera, float x, float y)
{
    if (camera->locked) {
        return;
    }

    camera->yaw += x;
    camera->pitch -= y;

    if (camera->pitch > 89.0) {
        camera->pitch = 89.0;
    } else if (camera->pitch < -89.0) {
        camera->pitch = -89.0;
    }

    V3 dir;
    dir.x = -cos(radians(camera->yaw)) * cos(radians(camera->pitch));
    dir.z = sin(radians(camera->pitch));
    dir.y = sin(radians(camera->yaw)) * cos(radians(camera->pitch));
    camera->front = norm(dir);
}
