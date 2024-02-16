#version 440
#extension GL_ARB_bindless_texture: require

#define SHADOW_BIAS 0.002
#define MAX_SPOTLIGHTS 6

in vec3 world_pos;
in vec2 uv;
in vec3 norm;
in vec3 color;
flat in uvec2 base_color;

in vec4 sl_light_space_pos[MAX_SPOTLIGHTS];

uniform vec3 camera_pos;

uniform uint sl_count;
uniform uvec2 sl_shadowmap[MAX_SPOTLIGHTS];
uniform vec3 sl_pos[MAX_SPOTLIGHTS];
uniform vec3 sl_dir[MAX_SPOTLIGHTS];
uniform float sl_fov[MAX_SPOTLIGHTS];

out vec4 out_Color;

vec3 l = normalize(vec3(1, 2, 3));

float shadow_calc(vec4 light_space_pos, sampler2D shadowmap) {
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    float closest = texture(shadowmap, proj_coords.xy).r;
    float curr = proj_coords.z;
    float shadow = curr - SHADOW_BIAS < closest ? 1.0 : 0.0;

    return shadow;
}

void main() {
    vec3 n = normalize(norm);
    vec3 v = normalize(camera_pos - world_pos);

    out_Color = texture(sampler2D(base_color), uv);
    out_Color.rgb *= color;

    vec3 ambient = vec3(0.1);
    float diffuse_int = clamp(dot(n, l), 0, 1);
    diffuse_int = step(0.1, diffuse_int);
    vec3 diffuse = vec3(1) * diffuse_int;
    float specular_int = 0;
    // float specular_int = clamp(dot(-reflect(l, n), v), 0, 1);
    // specular_int = step(0.98, specular_int);
    vec3 specular = vec3(1) * specular_int;
    vec3 light = ambient + 0.6 * diffuse + 0.5 * specular;

    for (uint i = 0; i < sl_count; ++i) {
        sampler2D shadowmap = sampler2D(sl_shadowmap[i]);
        vec3 pos = sl_pos[i];
        vec3 dir = sl_dir[i];
        float fov = sl_fov[i];

        vec3 side = vec3(-dir.y, dir.x, dir.z);
        vec3 left = normalize(fov * side + (1 - fov) * dir);

        if (dot(dir, normalize(world_pos - pos)) > dot(dir, left)) {
            vec3 light_color = vec3(10.0, 1.4, 1.4) * clamp(dot(normalize(pos - world_pos), n), 0, 1);
            light.rgb += light_color * shadow_calc(sl_light_space_pos[i], shadowmap);
        }
    }

    out_Color.rgb *= light;
}
