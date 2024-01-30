#version 440

in vec2 uv;
in vec2 norm;
in vec3 color;

uniform sampler2D base_color;

out vec4 out_Color;

void main() {
    out_Color = vec4(color, 1);
}
