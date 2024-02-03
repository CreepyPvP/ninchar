#version 440

in vec2 uv;

uniform sampler2D color;

out vec4 out_Color;

void main() {
    out_Color = texture(color, uv);
}
