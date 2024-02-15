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

    vec3 ambient = vec3(0.3);
    vec3 diffuse = vec3(clamp(dot(n, l), 0, 1));
    vec3 specular = vec3(clamp(dot(-reflect(l, n), v), 0, 1));
    vec3 light = ambient + diffuse + specular;

    out_Color = vec4(0.2, 0.1, 0.1, 1);
    out_Color.rgb *= light;
}
