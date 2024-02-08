#version 440

in vec3 world_pos;
in vec2 uv;
in vec3 norm;
in vec3 color;

uniform sampler2D base_color;

uniform vec3 sl_pos;
uniform vec4 sl_dir;

out vec4 out_Color;

vec3 l = normalize(vec3(1, 2, 3));

void main() {
    vec3 n = normalize(norm);

    out_Color = texture(base_color, uv);
    out_Color.rgb *= color;

    vec3 light = vec3(0.3 + 0.5 * clamp(dot(n, l), 0, 1));

    float fov = sl_dir.w;
    vec3 sl_side = vec3(-sl_dir.y, sl_dir.x, sl_dir.z);
    vec3 sl_left = normalize(fov * sl_side + (1 - fov) * sl_dir.xyz);
    if (dot(sl_dir.xyz, normalize(world_pos - sl_pos)) > dot(sl_dir.xyz, sl_left)) {
        light += vec3(2.0, 1.4, 1.4) * clamp(dot(normalize(sl_pos - world_pos), n), 0, 1);
    }

    out_Color.rgb *= light;
}
