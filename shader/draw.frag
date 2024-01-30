#version 440

in vec2 uv;
in vec2 norm;
in vec3 color;

uniform sampler2D base_color;

out vec4 out_Color;

void main() {
    out_Color = texture(base_color, uv);
    out_Color.rgb *= color;
}
