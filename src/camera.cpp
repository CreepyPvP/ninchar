#include "include/camera.h"

#include <math.h>

#define CAMERA_SPEED 5

void init_camera(Camera* camera, V3 pos, V3 front, V3 right)
{
    camera->pos = pos;
    camera->front = front;
    camera->right = right;
}

void update_camera(Camera* camera, u8 buttons_pressed, float delta)
{
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

    camera->pos.x += camera->front.x * movement.x * delta * CAMERA_SPEED;
    camera->pos.y += camera->front.y * movement.x * delta * CAMERA_SPEED;
    camera->pos.z += camera->front.z * movement.x * delta * CAMERA_SPEED;
    camera->pos.x += camera->right.x * movement.y * delta * CAMERA_SPEED;
    camera->pos.y += camera->right.y * movement.y * delta * CAMERA_SPEED;
    camera->pos.z += camera->right.z * movement.y * delta * CAMERA_SPEED;
};
