#include "include/renderer.h"

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include "include/game_math.h"


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

    commands.width = width;
    commands.height = height;
    commands.white = white;

    commands.active_group = NULL;

    return commands;
}

RenderGroup render_group(CommandBuffer* commands, Mat4 proj, bool lit, bool culling)
{
    RenderGroup group;
    group.commands = commands;
    group.current_draw = NULL;
    group.settings.proj = proj;
    group.settings.lit = lit;
    group.settings.culling = culling;
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

CommandEntryDraw* get_current_draw(RenderGroup* group, u32 quad_count)
{
    CommandBuffer* commands = group->commands;
    if (!group->current_draw || group->commands->active_group != group) {
        group->current_draw = (CommandEntryDraw*) push_entry(commands, sizeof(CommandEntryDraw));

        group->current_draw->header.type = EntryType_Draw;
        group->current_draw->quad_offset = commands->quad_count;
        group->current_draw->quad_count = 0;
        group->current_draw->settings = group->settings;

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
    CommandEntryDraw* draw = group->current_draw;
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
    CommandEntryDraw* entry = get_current_draw(group, 6);
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

void push_line(RenderGroup* group, V3 start, V3 end, V3 color)
{
    CommandEntryDraw* entry = get_current_draw(group, 1);

    // TODO: Orient line so it always faces the camera
    // TODO: Apply "lit" renderer setting correctly in the backend
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

void free_texture_load_op(TextureLoadOp* load_op)
{
    stbi_image_free(load_op->data);
}
