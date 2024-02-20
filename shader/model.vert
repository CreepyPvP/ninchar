#version 440

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec3 aColor;

uniform mat4 proj;
uniform mat4 trans;

out vec3 world_pos;
out vec2 uv;
out vec3 norm;
out vec3 base_color;

void main() {
    uv = aUv;
    norm = normalize(aNorm);
    base_color = aColor;
    vec4 pos = trans * vec4(aPos, 1);
    world_pos = pos.xyz / pos.w;

    gl_Position = proj * pos;
}
