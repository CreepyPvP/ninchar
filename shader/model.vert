layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec3 aColor;
#ifdef SKELETON
layout(location = 4) in ivec4 aBoneIds; 
layout(location = 5) in vec4 aWeights;

const int MAX_BONES = 50;
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
#ifdef SKELETON
    vec4 pos = vec4(0);
    norm = vec3(0);
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if(aBoneIds[i] == -1) {
            continue;
        }

        if(aBoneIds[i] >= MAX_BONES) {
            break;
        }
        vec4 local_pos = bone_trans[aBoneIds[i]] * vec4(aPos, 1);
        pos += local_pos * aWeights[i];
        vec3 local_norm = mat3(bone_trans[aBoneIds[i]]) * aNorm;
        norm += local_norm * aWeights[i];
    }
#else
    vec4 pos = vec4(aPos, 1);
    norm = normalize(trans * vec4(aNorm, 0)).xyz;
#endif

    uv = aUv;
    base_color = aColor;
    pos = trans * pos;
    world_pos = pos.xyz / pos.w;

    gl_Position = proj * pos;
}
