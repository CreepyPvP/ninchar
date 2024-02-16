#version 440

in vec3 world_pos;
in vec2 uv;
in vec3 norm;

uniform vec3 camera_pos;

out vec4 out_Color;

vec3 l = normalize(vec3(1, 2, 3));

void main() {
    vec3 n = normalize(norm);
    vec3 v = normalize(camera_pos - world_pos);

    vec3 ambient = vec3(0.2);
    float diffuse_int = clamp(dot(n, l), 0, 1);
    diffuse_int = step(0.1, diffuse_int);
    vec3 diffuse = vec3(1) * diffuse_int;
    float specular_int = clamp(dot(-reflect(l, n), v), 0, 1);
    specular_int = step(0.98, specular_int);
    vec3 specular = vec3(1) * specular_int;
    vec3 light = ambient + diffuse + specular;

    out_Color = vec4(0.2, 0.1, 0.1, 1);
    out_Color.rgb *= light;
}
