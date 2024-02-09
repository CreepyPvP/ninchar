#version 440

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec3 aColor;

uniform mat4 proj;

uniform mat4 light_space;

out vec3 world_pos;
out vec2 uv;
out vec3 norm;
out vec3 color;

out vec4 light_space_pos;

void main() {
    color = aColor;
    uv = aUv;
    norm = normalize(aNorm);
    world_pos = aPos;
    light_space_pos = light_space * vec4(aPos, 1);
    gl_Position = proj * vec4(aPos, 1);
}
