#include "include/renderer.h"

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include "include/game_math.h"
#include "include/util.h"

#define MAX_MODEL_VERT 10000
#define MAX_MODEL_INDEX 20000


CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 quad_cap, Vertex* vert_buffer, TextureHandle* texture_buffer,
                             u32 width, u32 height, TextureHandle white)
{
    CommandBuffer commands;
    commands.entry_buffer = entry_buffer;
    commands.entry_cap = entry_cap;
    commands.entry_size = 0;

    commands.vert_buffer = vert_buffer;
    commands.texture_buffer = texture_buffer;
    commands.quad_cap = quad_cap;
    commands.quad_count = 0;

    commands.settings.width = width;
    commands.settings.height = height;
    commands.white = white;

    commands.active_group = NULL;

    return commands;
}

RenderGroup render_group(CommandBuffer* commands, Mat4 proj, bool lit, bool culling)
{
    RenderGroup group;
    group.commands = commands;
    group.current_draw = NULL;
    group.setup.proj = proj;
    group.setup.lit = lit;
    group.setup.culling = culling;
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
        group->current_draw->quad_offset = commands->quad_count;
        group->current_draw->quad_count = 0;
        group->current_draw->setup = group->setup;

        group->commands->active_group = group;
    }

    if (commands->quad_count + quad_count > commands->quad_cap) {
        printf("Warning: Vertex buffer size exceede\n");
        return NULL;
    }

    return group->current_draw;
}

void push_quad(RenderGroup* group, 
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

    u32 vcurr = commands->quad_count * 4;
    commands->vert_buffer[vcurr + 0].pos = p1;
    commands->vert_buffer[vcurr + 0].uv = uv1;
    commands->vert_buffer[vcurr + 0].norm = norm;
    commands->vert_buffer[vcurr + 0].color = color;
    commands->vert_buffer[vcurr + 1].pos = p2;
    commands->vert_buffer[vcurr + 1].uv = uv2;
    commands->vert_buffer[vcurr + 1].norm = norm;
    commands->vert_buffer[vcurr + 1].color = color;
    commands->vert_buffer[vcurr + 2].pos = p3;
    commands->vert_buffer[vcurr + 2].uv = uv3;
    commands->vert_buffer[vcurr + 2].norm = norm;
    commands->vert_buffer[vcurr + 2].color = color;
    commands->vert_buffer[vcurr + 3].pos = p4;
    commands->vert_buffer[vcurr + 3].uv = uv4;
    commands->vert_buffer[vcurr + 3].norm = norm;
    commands->vert_buffer[vcurr + 3].color = color;

    commands->texture_buffer[commands->quad_count] = texture;
    
    ++commands->quad_count;
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

    push_quad(group, 
              p1, v2(0, 0),
              p2, v2(0, 1),
              p4, v2(1, 0),
              p3, v2(1, 1),
              v3(0, 0, 1), texture, color);

    push_quad(group, 
              p8, v2(0, 0),
              p7, v2(0, 1),
              p5, v2(1, 0),
              p6, v2(1, 1),
              v3(0, 0, -1), texture, color);

    push_quad(group, 
              p8, v2(0, 0),
              p4, v2(0, 1),
              p7, v2(1, 0),
              p3, v2(1, 1),
              v3(1, 0, 0), texture, color);

    push_quad(group, 
              p6, v2(0, 0),
              p2, v2(0, 1),
              p5, v2(1, 0),
              p1, v2(1, 1),
              v3(-1, 0, 0), texture, color);

    push_quad(group, 
              p7, v2(0, 0),
              p3, v2(0, 1),
              p6, v2(1, 0),
              p2, v2(1, 1),
              v3(0, 1, 0), texture, color);

    push_quad(group, 
              p5, v2(0, 0),
              p1, v2(0, 1),
              p8, v2(1, 0),
              p4, v2(1, 1),
              v3(0, -1, 0), texture, color);

}

void push_model(RenderGroup* group, V3 pos, ModelHandle handle)
{
    CommandBuffer* commands = group->commands;
    CommandEntryDrawModel* draw = (CommandEntryDrawModel*) push_entry(commands, sizeof(CommandEntryDrawModel));

    draw->header.type = EntryType_DrawModel;
    draw->model = handle;
    draw->setup = group->setup;
    draw->trans = mat4(pos);
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

    push_quad(group, 
              p0, v2(0, 0),
              p1, v2(0, 1),
              p2, v2(1, 0),
              p3, v2(1, 1),
              v3(0, 0, 1), group->commands->white, color);
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

ModelLoadOp model_load_op(ModelHandle* handle, const char* path, Arena* arena)
{
    u32 vert_stride = sizeof(u32) * 3;

    const char* file_content = read_file(path, NULL, arena);
    const char** ptr = &file_content;

    u32 vert_count = 0;
    u32 index_count = 0;
    u8* vert_buffer = (u8*) push_size(arena, vert_stride * MAX_MODEL_VERT);
    u32* index_buffer = (u32*) push_size(arena, sizeof(u32) * MAX_MODEL_INDEX);

    while (true) {
        if (prefix("v", ptr)) {
            skip_whitespaces(ptr);
            float* current = (float*) (vert_buffer + vert_stride * vert_count);

            // NOTE: Flipped to match game coordiante system
            current[0] = read_float(ptr);
            skip_whitespaces(ptr);
            current[2] = read_float(ptr);
            skip_whitespaces(ptr);
            current[1] = read_float(ptr);
            next_line(ptr);

            ++vert_count;
            assert(vert_count <= MAX_MODEL_VERT);
        } else if (prefix("f", ptr)) {
            skip_whitespaces(ptr);
            u32* current = index_buffer + index_count;

            current[0] = read_int(ptr) - 1;
            skip_whitespaces(ptr);
            current[1] = read_int(ptr) - 1;
            skip_whitespaces(ptr);
            current[2] = read_int(ptr) - 1;
            next_line(ptr);

            index_count += 3;
            assert(index_count <= MAX_MODEL_INDEX);
        } else {
            break;
        }
    }

    ModelLoadOp load;
    load.vert_buffer = vert_buffer;
    load.vert_count = vert_count;
    load.index_buffer = index_buffer;
    load.index_count = index_count;
    load.vert_stride = vert_stride;
    load.handle = handle;
    return load;
}

void free_texture_load_op(TextureLoadOp* load_op)
{
    stbi_image_free(load_op->data);
}
