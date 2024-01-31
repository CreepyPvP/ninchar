#ifndef CAMERA_H
#define CAMERA_H

#include "include/types.h"

struct Camera
{
    V3 pos;
    V3 front;
    V3 right;
};

void init_camera(Camera* camera, V3 pos, V3 front, V3 right);
void update_camera(Camera* camera, u8 buttons_pressed, float delta);

#endif
