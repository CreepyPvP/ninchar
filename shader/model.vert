#version 440

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;

uniform mat4 proj;
uniform mat4 trans;

out vec2 uv;
out vec3 norm;

void main() {
    uv = aUv;
    norm = normalize(aNorm);
    gl_Position = proj * trans * vec4(aPos, 1);
}
