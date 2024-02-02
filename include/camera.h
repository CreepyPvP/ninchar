#ifndef CAMERA_H
#define CAMERA_H

#include "include/types.h"

struct Camera
{
    V3 pos;
    V3 front;
    float yaw;
    float pitch;

    bool locked;
};

void init_camera(Camera* camera, V3 pos, V3 front);
void update_camera(Camera* camera, u8 buttons_pressed, float delta);
void update_camera_mouse(Camera* camera, float x, float y);

#endif
