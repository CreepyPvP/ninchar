layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec3 aColor;
#ifdef SKELETON
layout(location = 4) in ivec4 aBoneIds; 
layout(location = 5) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 3;
uniform mat4 bone_trans[MAX_BONES];
#endif

uniform mat4 proj;
uniform mat4 trans;

out vec3 world_pos;
out vec2 uv;
out vec3 norm;
out vec3 base_color;

void main() {
    uv = aUv;
    base_color = aColor;
    vec4 pos = vec4(aPos, 1);
    vec4 n = vec4(aNorm, 0);
#ifdef SKELETON
    mat4 bone = bone_trans[aBoneIds[0]] * aWeights[0];
    bone +=     bone_trans[aBoneIds[1]] * aWeights[1];
    bone +=     bone_trans[aBoneIds[2]] * aWeights[2];

    if (aBoneIds[0] == 1 || aBoneIds[1] == 1 || aBoneIds[2] == 1) {
        base_color = vec3(0, 1, 0);
    }

    pos = bone * pos;
    n = bone * n;
#endif
    pos = trans * pos;
    world_pos = pos.xyz / pos.w;
    norm = normalize(trans * n).xyz;

    gl_Position = proj * pos;
}
