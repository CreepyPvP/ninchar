#include "include/renderer.h"

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include "include/game_math.h"
#include "include/util.h"
#include "include/profiler.h"

#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MAX_MODEL_VERT 10000
#define MAX_MODEL_INDEX 20000


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, u32 vert_cap, Vertex* vert_buffer, 
                             u32 width, u32 height, TextureHandle white,
                             Mat4 proj, V3 camera_pos, V3 camera_up, V3 camera_right)
{
    CommandBuffer commands;
    commands.entry_buffer = entry_buffer;
    commands.entry_cap = entry_cap;
    commands.entry_size = 0;

    commands.vert_buffer = vert_buffer;
    commands.vert_cap = vert_cap;
    commands.vert_count = 0;

    commands.settings.width = width;
    commands.settings.height = height;
    commands.white = white;
    commands.active_group = NULL;

    commands.proj = proj;
    commands.camera_pos = camera_pos;
    commands.camera_up = camera_up;
    commands.camera_right = camera_right;

    return commands;
}

// TODO: Replace params with flags
RenderGroup render_group(CommandBuffer* commands, u32 flags)
{
    RenderGroup group = {};
    group.commands = commands;
    group.current_draw = NULL;
    group.setup.flags = flags;
    return group;
}

u8* push_entry(CommandBuffer* commands, u32 size)
{
    if (commands->entry_size + size > commands->entry_cap) {
        printf("Command buffer size exceeded\n");
        return NULL;
    }

    u8* entry = commands->entry_buffer + commands->entry_size;
    commands->entry_size += size;
    commands->active_group = NULL;
    return entry;
}

void push_clear(CommandBuffer* commands, V3 color)
{
    CommandEntryClear* clear = (CommandEntryClear*) push_entry(commands, sizeof(CommandEntryClear));
    if (!clear) {
        return;
    }

    clear->header.type = EntryType_Clear;
    clear->color = color;
}

CommandEntryDrawQuads* get_current_draw(RenderGroup* group, u32 quad_count)
{
    CommandBuffer* commands = group->commands;
    if (!group->current_draw || group->commands->active_group != group) {
        group->current_draw = (CommandEntryDrawQuads*) push_entry(commands, sizeof(CommandEntryDrawQuads));

        group->current_draw->header.type = EntryType_DrawQuads;
        group->current_draw->vert_offset = commands->vert_count;
        group->current_draw->quad_count = 0;
        group->current_draw->setup = group->setup;

        group->commands->active_group = group;
    }

    assert(commands->vert_count + quad_count * 4 <= commands->vert_cap);

    return group->current_draw;
}

void push_rect(RenderGroup* group, 
               V3 p1, V2 uv1,
               V3 p2, V2 uv2,
               V3 p3, V2 uv3,
               V3 p4, V2 uv4,
               V3 norm, TextureHandle texture, V3 color)
{
    CommandBuffer* commands = group->commands;
    CommandEntryDrawQuads* draw = group->current_draw;
    assert(draw);

    ++draw->quad_count;

    u32 vcurr = commands->vert_count;
    commands->vert_buffer[vcurr + 0].pos = p1;
    commands->vert_buffer[vcurr + 0].uv = uv1;
    commands->vert_buffer[vcurr + 0].norm = norm;
    commands->vert_buffer[vcurr + 0].color = color;
    commands->vert_buffer[vcurr + 0].texture = texture.id;
    commands->vert_buffer[vcurr + 1].pos = p2;
    commands->vert_buffer[vcurr + 1].uv = uv2;
    commands->vert_buffer[vcurr + 1].norm = norm;
    commands->vert_buffer[vcurr + 1].color = color;
    commands->vert_buffer[vcurr + 1].texture = texture.id;
    commands->vert_buffer[vcurr + 2].pos = p3;
    commands->vert_buffer[vcurr + 2].uv = uv3;
    commands->vert_buffer[vcurr + 2].norm = norm;
    commands->vert_buffer[vcurr + 2].color = color;
    commands->vert_buffer[vcurr + 2].texture = texture.id;
    commands->vert_buffer[vcurr + 3].pos = p4;
    commands->vert_buffer[vcurr + 3].uv = uv4;
    commands->vert_buffer[vcurr + 3].norm = norm;
    commands->vert_buffer[vcurr + 3].color = color;
    commands->vert_buffer[vcurr + 3].texture = texture.id;

    commands->vert_count += 4;
}

void push_cube(RenderGroup* group, V3 pos, V3 radius, TextureHandle texture, V3 color)
{
    CommandEntryDrawQuads* entry = get_current_draw(group, 6);
    if (!entry) {
        return;
    }

    V3 p1 = v3(pos.x - radius.x, pos.y - radius.y, pos.z + radius.z);
    V3 p2 = v3(pos.x - radius.x, pos.y + radius.y, pos.z + radius.z);
    V3 p3 = v3(pos.x + radius.x, pos.y + radius.y, pos.z + radius.z);
    V3 p4 = v3(pos.x + radius.x, pos.y - radius.y, pos.z + radius.z);
    V3 p5 = v3(pos.x - radius.x, pos.y - radius.y, pos.z - radius.z);
    V3 p6 = v3(pos.x - radius.x, pos.y + radius.y, pos.z - radius.z);
    V3 p7 = v3(pos.x + radius.x, pos.y + radius.y, pos.z - radius.z);
    V3 p8 = v3(pos.x + radius.x, pos.y - radius.y, pos.z - radius.z);

    push_rect(group, 
              p1, v2(0, 0),
              p2, v2(0, 1),
              p4, v2(1, 0),
              p3, v2(1, 1),
              v3(0, 0, 1), texture, color);

    push_rect(group, 
              p8, v2(0, 0),
              p7, v2(0, 1),
              p5, v2(1, 0),
              p6, v2(1, 1),
              v3(0, 0, -1), texture, color);

    push_rect(group, 
              p8, v2(0, 0),
              p4, v2(0, 1),
              p7, v2(1, 0),
              p3, v2(1, 1),
              v3(1, 0, 0), texture, color);

    push_rect(group, 
              p6, v2(0, 0),
              p2, v2(0, 1),
              p5, v2(1, 0),
              p1, v2(1, 1),
              v3(-1, 0, 0), texture, color);

    push_rect(group, 
              p7, v2(0, 0),
              p3, v2(0, 1),
              p6, v2(1, 0),
              p2, v2(1, 1),
              v3(0, 1, 0), texture, color);

    push_rect(group, 
              p5, v2(0, 0),
              p1, v2(0, 1),
              p8, v2(1, 0),
              p4, v2(1, 1),
              v3(0, -1, 0), texture, color);

}

void push_model(RenderGroup* group, ModelHandle handle, V3 pos, V3 scale)
{
    CommandBuffer* commands = group->commands;
    CommandEntryDrawModel* draw = (CommandEntryDrawModel*) push_entry(commands, sizeof(CommandEntryDrawModel));

    draw->header.type = EntryType_DrawModel;
    draw->model = handle;
    draw->setup = group->setup;
    draw->trans = mat4(pos, scale);
}

void push_rigged_model(RenderGroup* group, RiggedModelHandle* handle, Mat4* pose, V3 pos, V3 scale)
{
    CommandBuffer* commands = group->commands;
    CommandEntryDrawRiggedModel* draw = (CommandEntryDrawRiggedModel*) 
        push_entry(commands, sizeof(CommandEntryDrawRiggedModel));

    draw->header.type = EntryType_DrawRiggedModel;
    draw->model = handle->model;
    draw->setup = group->setup;
    draw->trans = mat4(pos, scale);

    draw->bone_count = handle->skeleton.bone_count;
    draw->bone_trans = pose;
}

void push_debug_pose(RenderGroup* group, Skeleton* sk, Mat4* pose, V3 pos, V3 scale)
{
#ifdef DEBUG
    CommandEntryDrawQuads* entry = get_current_draw(group, sk->bone_count + 1);
    if (!entry) {
        return;
    }

    float size = 0.025;

    Mat4 trans = mat4(pos, scale);
    V3 right = group->commands->camera_right;
    V3 up = group->commands->camera_up;

    for (u32 i = 0; i < sk->bone_count; ++i) {
        Mat4 bone_trans = trans * pose[i] * glm::inverse(sk->bone[i].offset);

        glm::vec4 tmp = bone_trans * glm::vec4(0, 0, 0, 1); 

        // TODO: Need to divide by tmp.w? - No? 
        V3 pos = v3(tmp.x, tmp.y, tmp.z);

        V3 p0 = v3(pos.x - size * right.x, pos.y - size * right.y, pos.z - size * right.z);
        V3 p1 = v3(pos.x + size * up.x, pos.y + size * up.y, pos.z + size * up.z);
        V3 p2 = v3(pos.x - size * up.x, pos.y - size * up.y, pos.z - size * up.z);
        V3 p3 = v3(pos.x + size * right.x, pos.y + size * right.y, pos.z + size * right.z);

        push_rect(group, 
                  p0, v2(0, 0),
                  p1, v2(0, 1),
                  p2, v2(1, 0),
                  p3, v2(1, 1),
                  v3(0, 0, 1), group->commands->white, v3(0, 0, 1));
    }

    {
        glm::vec4 tmp = trans * glm::vec4(0, 0, 0, 1); 

        // TODO: Need to divide by tmp.w? - No? 
        V3 pos = v3(tmp.x, tmp.y, tmp.z);

        V3 p0 = v3(pos.x - size * right.x, pos.y - size * right.y, pos.z - size * right.z);
        V3 p1 = v3(pos.x + size * up.x, pos.y + size * up.y, pos.z + size * up.z);
        V3 p2 = v3(pos.x - size * up.x, pos.y - size * up.y, pos.z - size * up.z);
        V3 p3 = v3(pos.x + size * right.x, pos.y + size * right.y, pos.z + size * right.z);

        push_rect(group, 
                  p0, v2(0, 0),
                  p1, v2(0, 1),
                  p2, v2(1, 0),
                  p3, v2(1, 1),
                  v3(0, 0, 1), group->commands->white, v3(0, 1, 0));
    }
#endif
}

void push_line(RenderGroup* group, V3 start, V3 end, V3 color)
{
    CommandEntryDrawQuads* entry = get_current_draw(group, 1);

    V3 up = cross(group->commands->camera_up, group->commands->camera_right);
    float width = 0.025;

    V3 dir = norm(v3(end.x - start.x, end.y - start.y, end.z - start.z));
    V3 side = norm(cross(dir, up));

    V3 p0 = v3(start.x - width * side.x, start.y - width * side.y, start.z - width * side.z);
    V3 p1 = v3(start.x + width * side.x, start.y + width * side.y, start.z + width * side.z);
    V3 p2 = v3(end.x - width * side.x, end.y - width * side.y, end.z - width * side.z);
    V3 p3 = v3(end.x + width * side.x, end.y + width * side.y, end.z + width * side.z);

    push_rect(group, 
              p0, v2(0, 0),
              p1, v2(0, 1),
              p2, v2(1, 0),
              p3, v2(1, 1),
              v3(0, 0, 1), group->commands->white, color);
}

void push_spotlight(CommandBuffer* commands, V3 pos, V3 dir, float fov, float far_plane)
{
    CommandEntryPushLight* light =  (CommandEntryPushLight*) push_entry(commands, sizeof(CommandEntryPushLight));
    light->header.type = EntryType_PushLight;
    light->pos = pos;
    light->dir = dir;
    light->fov = fov;

    float near_plane = 0.25;
    //float far_plane = 20;
    Mat4 proj = glm::perspective(45.0f, 1.0f, near_plane, far_plane);
    Mat4 view = glm::lookAt(glm::vec3(pos.x, pos.y, pos.z), 
                            glm::vec3(pos.x + dir.x, pos.y + dir.y, pos.z + dir.z), 
                            glm::vec3( 0.0f, 0.0f,  1.0f));     
    light->light_space = proj * view;
}

TextureLoadOp texture_load_op(TextureHandle* handle, const char* path)
{
    stbi_set_flip_vertically_on_load(true);
    TextureLoadOp load_op;
    load_op.handle = handle;
    load_op.data = stbi_load(path, &load_op.width, &load_op.height, 
                             &load_op.num_channels, 0);
    if (!load_op.data) {
        assert(0 && "Failed to load texture");
    }

    return load_op;
}

void free_texture_load_op(TextureLoadOp* load_op)
{
    stbi_image_free(load_op->data);
}

Mat4 read_assimp_mat(aiMatrix4x4 mat)
{
    Mat4 to;
    to[0][0] = mat.a1; to[1][0] = mat.a2; to[2][0] = mat.a3; to[3][0] = mat.a4;
    to[0][1] = mat.b1; to[1][1] = mat.b2; to[2][1] = mat.b3; to[3][1] = mat.b4;
    to[0][2] = mat.c1; to[1][2] = mat.c2; to[2][2] = mat.c3; to[3][2] = mat.c4;
    to[0][3] = mat.d1; to[1][3] = mat.d2; to[2][3] = mat.d3; to[3][3] = mat.d4;
    return to;
}

void process_scene_node(aiNode *node, const aiScene *scene, ModelLoadOp* load_op, Skeleton* sk,
                        Arena* tmp, Arena* assets)
{
    for (u32 i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
        MeshInfo info = {};

        info.vertex_count = mesh->mNumVertices;
        info.vertex_buffer = (MeshVertex*) push_size(tmp, sizeof(MeshVertex) * info.vertex_count);
        info.index_count = mesh->mNumFaces * 3;
        info.index_buffer = (u32*) push_size(tmp, sizeof(u32) * info.index_count);

        info.flags = 0;
        if (mesh->HasTextureCoords(0)) {
            info.flags |= MODEL_FLAGS_UV;
        }
        if (mesh->HasBones() && sk && assets) {
            info.flags |= MODEL_FLAGS_RIGGED;
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        V4 color = v4(0.6);
        aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, (aiColor4D*) &color);

        for (u32 i = 0; i < info.vertex_count; ++i) {
            MeshVertex vert;
            vert.pos = v3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vert.norm = v3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            vert.color = color.rgb;

            for (u32 i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                vert.bone_ids[i] = -1;
                vert.bone_weights[i] = 0;
            }

            if (info.flags & MODEL_FLAGS_UV) {
                vert.uv = v2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }

            info.vertex_buffer[i] = vert;
        }

        for (u32 i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            info.index_buffer[3 * i + 0] = face.mIndices[0];
            info.index_buffer[3 * i + 1] = face.mIndices[1];
            info.index_buffer[3 * i + 2] = face.mIndices[2];
        }  

        if (info.flags & MODEL_FLAGS_RIGGED) {
            for (u32 i = 0; i < mesh->mNumBones; ++i) {
                Str name = from_c_str(mesh->mBones[i]->mName.C_Str(), tmp);
                u32 bone_id;
                bool exists = false;
                for (u32 j = 0; j < sk->bone_count; ++j) {
                    if (str_equals(name, sk->bone[j].name)) {
                        exists = true;
                        bone_id = j;
                        break;
                    }
                }

                if (!exists) {
                    assert(sk->bone_count < sk->bone_cap);
                    bone_id = sk->bone_count;
                    sk->bone[bone_id].name = str_cpy(&name, assets);
                    sk->bone[bone_id].offset = read_assimp_mat(mesh->mBones[i]->mOffsetMatrix);
                    sk->bone_count++;
                }

                aiVertexWeight* weights = mesh->mBones[i]->mWeights;
                i32 weight_count = mesh->mBones[i]->mNumWeights;

                for (u32 j = 0; j < weight_count; ++j) {
                    u32 vert_id = weights[j].mVertexId;
                    float weight = weights[j].mWeight;

                    for (u32 k = 0; k < MAX_BONE_INFLUENCE; ++k) {
                        if (info.vertex_buffer[vert_id].bone_ids[k] < 0) {
                            info.vertex_buffer[vert_id].bone_ids[k] = bone_id;
                            info.vertex_buffer[vert_id].bone_weights[k] = weight;
                            break;
                        }
                    }
                }
            }
        }

        load_op->meshes[load_op->mesh_count] = info;
        ++load_op->mesh_count;
    }

    for (u32 i = 0; i < node->mNumChildren; ++i) {
        process_scene_node(node->mChildren[i], scene, load_op, sk, tmp, assets);
    }
}  

ModelLoadOp load_model(ModelHandle* handle, Skeleton* skeleton, const char* path, Arena* tmp, 
                       Arena* assets)
{
    Assimp::Importer importer;

    u32 flags = aiProcess_FlipUVs | aiProcess_FlipWindingOrder;
// #if 0
    flags |= aiProcess_Triangulate;
// #endif
    const aiScene* scene = importer.ReadFile(path, flags); 
    assert(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    ModelLoadOp load_op = {};
    load_op.handle = handle;
    load_op.mesh_cap = scene->mNumMeshes;
    load_op.meshes = (MeshInfo*) push_size(tmp, sizeof(MeshInfo) * load_op.mesh_cap);

    if (skeleton) {
        skeleton->inverse_trans = glm::inverse(read_assimp_mat(scene->mRootNode->mTransformation));
    }

    process_scene_node(scene->mRootNode, scene, &load_op, skeleton, tmp, assets);

    return load_op;
}

ModelLoadOp model_load_op(ModelHandle* handle, const char* path, Arena* tmp)
{
    return load_model(handle, NULL, path, tmp, NULL);
}

ModelLoadOp sk_model_load_op(RiggedModelHandle* handle, const char* path, 
                             Arena* tmp, Arena* assets)
{
    *handle = {};
    handle->skeleton.bone_cap = 64;
    handle->skeleton.bone = (BoneInfo*) 
        push_size(assets, sizeof(BoneInfo) * handle->skeleton.bone_cap);
    return load_model(&handle->model, &handle->skeleton, path, tmp, assets);
}

u32 count_nodes(aiNode* node)
{
    u32 count = 1 + node->mNumChildren;
    for (u32 i = 0; i < node->mNumChildren; ++i) {
        count += count_nodes(node->mChildren[i]);
    }
    return count;
}

void process_skeleton_node(aiNode* node, Animation* anim, Arena* assets, u32 index, u32* node_count)
{
    AnimationNode entry;
    entry.name = from_c_str(node->mName.C_Str(), assets);
    entry.first_child = *node_count;
    entry.child_count = node->mNumChildren;
    entry.trans = read_assimp_mat(node->mTransformation);
    entry.bone = -1;

    for (u32 i = 0; i < anim->bone_count; ++i) {
        if (str_equals(entry.name, anim->bone[i].name)) {
            entry.bone = i;
            break;
        }
    }

    anim->node[index] = entry;
    (*node_count) += node->mNumChildren;

    for (u32 i = 0; i < node->mNumChildren; ++i) {
        process_skeleton_node(node->mChildren[i], anim, assets, entry.first_child + i, node_count);
    }
}

Animation load_animation(const char* path, Arena* assets)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 0); 
    assert(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    aiAnimation* animation = scene->mAnimations[0];

    Animation anim = {};
    anim.duration = animation->mDuration;
    anim.tps = animation->mTicksPerSecond;
    anim.bone_count = animation->mNumChannels;
    anim.bone = (Bone*) push_size(assets, sizeof(Bone) * anim.bone_count);

    for (u32 i = 0; i < anim.bone_count; ++i) {
        aiNodeAnim* channel = animation->mChannels[i];
        anim.key_count += channel->mNumPositionKeys;
        anim.key_count += channel->mNumRotationKeys;
        anim.key_count += channel->mNumScalingKeys;
    }
    anim.key = (AnimationKey*) push_size(assets, sizeof(AnimationKey) * anim.key_count);

    u32 current_key = 0;
    for (u32 i = 0; i < anim.bone_count; ++i) {
        aiNodeAnim* channel = animation->mChannels[i];

        Bone bone = {};
        bone.name = from_c_str(channel->mNodeName.C_Str(), assets); 
        bone.key_offset = current_key;
        bone.key_count = channel->mNumPositionKeys + channel->mNumRotationKeys + 
            channel->mNumScalingKeys;

        for (u32 i = 0; i < channel->mNumPositionKeys; ++i) {
            AnimationKey key = {};
            key.type = KeyType_Pos;
            key.timestamp = channel->mPositionKeys[i].mTime;
            aiVector3D pos = channel->mPositionKeys[i].mValue;
            key.v3 = v3(pos.x, pos.y, pos.z);

            anim.key[current_key] = key;
            current_key++;
        }

        for (u32 i = 0; i < channel->mNumRotationKeys; ++i) {
            AnimationKey key = {};
            key.type = KeyType_Rot;
            key.timestamp = channel->mRotationKeys[i].mTime;
            float w = channel->mRotationKeys[i].mValue.w;
            float x = channel->mRotationKeys[i].mValue.x;
            float y = channel->mRotationKeys[i].mValue.y;
            float z = channel->mRotationKeys[i].mValue.z;
            key.rot = glm::quat(w, x, y, z);

            anim.key[current_key] = key;
            current_key++;
        }

        for (u32 i = 0; i < channel->mNumScalingKeys; ++i) {
            AnimationKey key = {};
            key.type = KeyType_Scale;
            key.timestamp = channel->mScalingKeys[i].mTime;

            aiVector3D scale = channel->mScalingKeys[i].mValue;
            key.v3 = v3(scale.x, scale.y, scale.z);

            anim.key[current_key] = key;
            current_key++;
        }

        anim.bone[i] = bone;
    }

    anim.node_count = count_nodes(scene->mRootNode);
    anim.node = (AnimationNode*) push_size(assets, sizeof(AnimationNode) * anim.node_count);
    u32 count = 1;
    process_skeleton_node(scene->mRootNode, &anim, assets, 0, &count);

    return anim;
}

Mat4* default_pose(Skeleton* skeleton, Arena* arena)
{
    Mat4* res = (Mat4*) push_size(arena, sizeof(Mat4) * skeleton->bone_count);

    for (u32 i = 0; i < skeleton->bone_count; ++i) {
        res[i] = glm::mat4(1);
    }

    return res;
}

void do_node_trans(Animation* anim, Skeleton* sk, u32 id, Mat4 parent, Mat4* final, float time)
{
    AnimationNode* node = anim->node + id;
    Mat4 trans = node->trans;
    if (node->bone >= 0) {
        Bone* bone = anim->bone + node->bone;
        
        AnimationKey* pos_from = NULL;
        AnimationKey* pos_to = NULL;
        AnimationKey* rot_from = NULL;
        AnimationKey* rot_to = NULL;
        AnimationKey* scale_from = NULL;
        AnimationKey* scale_to = NULL;

        for (u32 i = 0; i < bone->key_count; ++i) {
            u32 key_id = bone->key_offset + i;
            AnimationKey* key = anim->key + key_id;
            // TODO: Clean this up
            if (key->type == KeyType_Pos) {
                if (key->timestamp <= time && (!pos_from || pos_from->timestamp < key->timestamp)) {
                    pos_from = key;
                    continue;
                }

                if (key->timestamp >= time && (!pos_to || pos_to->timestamp > key->timestamp)) {
                    pos_to = key;
                    continue;
                }
            }

            if (key->type == KeyType_Rot) {
                if (key->timestamp <= time && (!rot_from || rot_from->timestamp < key->timestamp)) {
                    rot_from = key;
                    continue;
                }

                if (key->timestamp >= time && (!rot_to || rot_to->timestamp > key->timestamp)) {
                    rot_to = key;
                    continue;
                }
            }
            
            if (key->type == KeyType_Scale) {
                if (key->timestamp <= time && (!scale_from || scale_from->timestamp < key->timestamp)) {
                    scale_from = key;
                    continue;
                }

                if (key->timestamp >= time && (!scale_to || scale_to->timestamp > key->timestamp)) {
                    scale_to = key;
                    continue;
                }
            }
        }

        // TODO: Clean this part up
        Mat4 trans_pos;
        {
            V3 start_pos;
            V3 end_pos;
            float start;
            float end;

            if (pos_from) {
                start = pos_from->timestamp;
                start_pos = pos_from->v3;
            } else {
                start = 0;
                start_pos = v3(0);
            }

            if (pos_to) {
                end = pos_to->timestamp;
                end_pos = pos_to->v3;
            } else {
                end = anim->duration;
                end_pos = v3(0);
            }

            float t = (time - start) / (end - start);
            V3 pos = lerp(start_pos, end_pos, t);
            trans_pos = glm::translate(glm::mat4(1), glm::vec3(pos.x, pos.y, pos.z));
        }

        Mat4 trans_rot;
        {
            Quat start_rot;
            Quat end_rot;
            float start;
            float end;

            if (rot_from) {
                start = rot_from->timestamp;
                start_rot = rot_from->rot;
            }  else {
                start = 0;
                start_rot = glm::quat(1, 0, 0, 0);
            }

            if (rot_to) {
                end = rot_to->timestamp;
                end_rot = rot_to->rot;
            }  else {
                end = anim->duration;
                end_rot = glm::quat(1, 0, 0, 0);
            }

            float t = (time - start) / (end - start);
            Quat rot = glm::slerp(start_rot, end_rot, t);
            rot = glm::normalize(rot);
            trans_rot = glm::toMat4(rot);
        }

        Mat4 trans_scale;
        {
            V3 start_scale;
            V3 end_scale;
            float start;
            float end;
            
            if (scale_from) {
                start = scale_from->timestamp;
                start_scale = scale_from->v3;
            } else {
                start = 0;
                start_scale = v3(1);
            }

            if (scale_to) {
                end = scale_to->timestamp;
                end_scale = scale_to->v3;
            } else {
                end = anim->duration;
                end_scale = scale_to->v3;
            }

            float t = (time - start) / (end - start);
            V3 scale = lerp(start_scale, end_scale, t);
            trans_scale = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
        }

        trans = trans_pos * trans_rot * trans_scale;
    }

    Mat4 global_trans = parent * trans;

    for (u32 i = 0; i < sk->bone_count; ++i) {
        BoneInfo* info = sk->bone + i;
        if (str_equals(node->name, info->name)) {
            // TODO: Figure this out
            final[i] = /* sk->inverse_trans * */ global_trans * info->offset;
            break;
        }
    }

    for (u32 i = 0; i < node->child_count; ++i) {
        do_node_trans(anim, sk, node->first_child + i, global_trans, final, time);
    }
}

Mat4* interpolate_pose(Animation* animation, Skeleton* skeleton, Arena* arena, float t)
{
    LogEntryInfo info = start_log(LogTarget_InterpolatePose);

    Mat4* res = (Mat4*) push_size(arena, sizeof(Mat4) * skeleton->bone_count);
    do_node_trans(animation, skeleton, 0, glm::mat4(1), res, t);

    end_log(info);

    return res;
}
