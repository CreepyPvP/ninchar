#version 440

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec3 aColor;

uniform mat4 proj;

out vec3 world_pos;
out vec2 uv;
out vec3 norm;
out vec3 color;

void main() {
    color = aColor;
    uv = aUv;
    norm = normalize(aNorm);
    world_pos = aPos;
    gl_Position = proj * vec4(aPos, 1);
}
