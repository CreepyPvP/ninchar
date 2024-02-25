#include "include/renderer.h"

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include "include/game_math.h"
#include "include/util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MAX_MODEL_VERT 10000
#define MAX_MODEL_INDEX 20000


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 vert_cap, Vertex* vert_buffer, 
                             u32 width, u32 height, TextureHandle white, V3 camera_pos)
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
    commands.camera_pos = camera_pos;

    return commands;
}

RenderGroup render_group(CommandBuffer* commands, Mat4 proj, bool lit, bool culling, 
                         bool shadow_caster)
{
    RenderGroup group = {};
    group.commands = commands;
    group.current_draw = NULL;
    group.setup.proj = proj;
    group.setup.lit = lit;
    group.setup.culling = culling;
    group.setup.shadow_caster = shadow_caster;
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

void push_line(RenderGroup* group, V3 start, V3 end, V3 color)
{
    CommandEntryDrawQuads* entry = get_current_draw(group, 1);

    // TODO: Orient line so it always faces the camera
    V3 up = v3(0, 0, 1);
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

    float near_plane = 0.4;
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

void process_scene_node(aiNode *node, const aiScene *scene, ModelLoadOp* load_op, Skeleton* sk,
                        Arena* tmp)
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
        if (mesh->HasBones() && sk) {
            info.flags |= MODEL_FLAGS_RIGGED;
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        V4 color = v4(0.6);
        aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, (aiColor4D*) &color);

        for (u32 i = 0; i < info.vertex_count; ++i) {
            MeshVertex vert;
            vert.pos = v3(mesh->mVertices[i].x, mesh->mVertices[i].z, mesh->mVertices[i].y);
            vert.norm = v3(mesh->mNormals[i].x, mesh->mNormals[i].z, mesh->mNormals[i].y);
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
                    sk->bone[bone_id].name = name;
                    // NOTE: Row major (assimp) needs to be converted to colum major (opengl)
                    Mat4* offset_t = (Mat4*) &mesh->mBones[i]->mOffsetMatrix;
                    sk->bone[bone_id].offset = glm::transpose(*offset_t);
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
        process_scene_node(node->mChildren[i], scene, load_op, sk, tmp);
    }
}  

ModelLoadOp load_model(ModelHandle* handle, Skeleton* skeleton, const char* path, Arena* tmp)
{
    Assimp::Importer importer;

    u32 flags = aiProcess_FlipUVs;
// #if 0
    flags |= aiProcess_Triangulate;
// #endif
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); 
    assert(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    ModelLoadOp load_op = {};
    load_op.handle = handle;
    load_op.mesh_cap = scene->mNumMeshes;
    load_op.meshes = (MeshInfo*) push_size(tmp, sizeof(MeshInfo) * load_op.mesh_cap);

    process_scene_node(scene->mRootNode, scene, &load_op, skeleton, tmp);

    return load_op;
}

ModelLoadOp model_load_op(ModelHandle* handle, const char* path, Arena* tmp)
{
    return load_model(handle, NULL, path, tmp);
}

ModelLoadOp sk_model_load_op(RiggedModelHandle* handle, const char* path, 
                             Arena* tmp, Arena* assets)
{
    *handle = {};
    handle->skeleton.bone_cap = 64;
    handle->skeleton.bone = (BoneInfo*) push_size(assets, sizeof(BoneInfo) * handle->skeleton.bone_cap);
    return load_model(&handle->model, &handle->skeleton, path, tmp);
}

void free_texture_load_op(TextureLoadOp* load_op)
{
    stbi_image_free(load_op->data);
}

Mat4* default_pose(RiggedModelHandle* handle, Arena* arena)
{
    Mat4* res = (Mat4*) push_size(arena, sizeof(Mat4) * handle->skeleton.bone_count);

    for (u32 i = 0; i < handle->skeleton.bone_count; ++i) {
        res[i] = handle->skeleton.bone[i].offset;
    }

    return res;
}
