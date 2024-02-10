#version 440
#extension GL_ARB_bindless_texture: require

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec3 aColor;
layout(location = 4) in uvec2 aBaseColor;

uniform mat4 proj;

uniform mat4 light_space;

out vec3 world_pos;
out vec2 uv;
out vec3 norm;
out vec3 color;
flat out uvec2 base_color;

out vec4 light_space_pos;

void main() {
    color = aColor;
    uv = aUv;
    norm = normalize(aNorm);
    world_pos = aPos;
    light_space_pos = light_space * vec4(aPos, 1);
    base_color = aBaseColor;
    gl_Position = proj * vec4(aPos, 1);
}
