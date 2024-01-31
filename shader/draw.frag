#version 440

in vec2 uv;
in vec3 norm;
in vec3 color;

uniform sampler2D base_color;

out vec4 out_Color;

vec3 l = normalize(vec3(3, 2, 1));

void main() {
    vec3 n = normalize(norm);

    out_Color = vec4(color, 1);

    float light = 0.5 + 0.5 * clamp(dot(n, l), 0, 1);
    out_Color.rgb *= light;
}
