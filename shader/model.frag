#version 440

in vec2 uv;
in vec3 norm;

out vec4 out_Color;

vec3 l = normalize(vec3(1, 2, 3));

void main() {
    vec3 n = normalize(norm);

    vec3 light = vec3(0.3 + 0.5 * clamp(dot(n, l), 0, 1));

    out_Color = vec4(0.1, 0.1, 0.1, 1);
    out_Color.rgb *= light;
}
