#version 440

in vec2 uv;
in vec3 norm;
in vec3 color;

uniform sampler2D base_color;

out vec4 out_Color;

vec3 l = normalize(vec3(1, 2, 3));

void main() {
    vec3 n = normalize(norm);

    out_Color = texture(base_color, uv);
    out_Color.rgb *= color;

    float light = 0.5 + 0.5 * clamp(dot(n, l), 0, 1);
    out_Color.rgb *= light;
}
