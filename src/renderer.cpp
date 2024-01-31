#include "include/renderer.h"

#include <stdio.h>

CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 vert_cap, Vertex* vert_buffer, 
                             u32 index_cap, u32* index_buffer)
{
    CommandBuffer commands;
    commands.entry_buffer = entry_buffer;
    commands.entry_cap = entry_cap;
    commands.entry_size = 0;
    commands.vert_buffer = vert_buffer;
    commands.vert_cap = vert_cap;
    commands.vert_count = 0;
    commands.index_buffer = index_buffer;
    commands.index_cap = index_cap;
    commands.index_count = 0;
    return commands;
}

RenderGroup render_group(CommandBuffer* commands, Mat4 proj)
{
    RenderGroup group;
    group.commands = commands;
    group.current_draw = NULL;
    group.proj = proj;
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
    CommandEntry_Clear* clear = (CommandEntry_Clear*) push_entry(commands, sizeof(CommandEntry_Clear));
    if (!clear) {
        return;
    }

    clear->header.type = EntryType_Clear;
    clear->color = color;
}

CommandEntry_Draw* get_current_draw(RenderGroup* group, u32 vert_count, u32 index_count)
{
    CommandBuffer* commands = group->commands;
    if (!group->current_draw) {
        group->current_draw = (CommandEntry_Draw*) push_entry(commands, sizeof(CommandEntry_Draw));

        group->current_draw->header.type = EntryType_Draw;
        group->current_draw->index_offset = commands->index_count;
        group->current_draw->index_count = 0;
        group->current_draw->proj = group->proj;
    }

    if (commands->vert_count + vert_count > commands->vert_cap) {
        printf("Warning: Vertex buffer size exceede\n");
        return NULL;
    }
    if (commands->index_count + index_count > commands->index_cap) {
        printf("Warning: Index buffer size exceede\n");
        return NULL;
    }

    return group->current_draw;
}

void push_quad(RenderGroup* group, 
               V3 p1, V2 uv1,
               V3 p2, V2 uv2,
               V3 p3, V2 uv3,
               V3 p4, V2 uv4,
               V3 norm, V3 color)
{
    CommandBuffer* commands = group->commands;
    CommandEntry_Draw* draw = group->current_draw;
    assert(draw);

    draw->index_count += 6;

    u32 vcurr = commands->vert_count;
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

    u32 icurr = commands->index_count;
    commands->index_buffer[icurr + 0] = 0 + vcurr;
    commands->index_buffer[icurr + 1] = 1 + vcurr;
    commands->index_buffer[icurr + 2] = 2 + vcurr;
    commands->index_buffer[icurr + 3] = 0 + vcurr;
    commands->index_buffer[icurr + 4] = 2 + vcurr;
    commands->index_buffer[icurr + 5] = 3 + vcurr;
    
    commands->index_count += 6;
    commands->vert_count += 4;
}

void push_cube(RenderGroup* group, V3 pos, V3 radius, V3 color)
{
    CommandEntry_Draw* entry = get_current_draw(group, 8, 12);
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
              p3, v2(1, 1),
              p4, v2(1, 0),
              v3(0, 0, 1), color);

    push_quad(group, 
              p8, v2(0, 0),
              p7, v2(0, 1),
              p6, v2(1, 1),
              p5, v2(1, 0),
              v3(0, 0, -1), color);

    push_quad(group, 
              p8, v2(0, 0),
              p4, v2(0, 1),
              p3, v2(1, 1),
              p7, v2(1, 0),
              v3(1, 0, 0), color);

    push_quad(group, 
              p6, v2(0, 0),
              p2, v2(0, 1),
              p1, v2(1, 1),
              p5, v2(1, 0),
              v3(-1, 0, 0), color);

    push_quad(group, 
              p7, v2(0, 0),
              p3, v2(0, 1),
              p2, v2(1, 1),
              p6, v2(1, 0),
              v3(0, 1, 0), color);

    push_quad(group, 
              p5, v2(0, 0),
              p1, v2(0, 1),
              p4, v2(1, 1),
              p8, v2(1, 0),
              v3(0, -1, 0), color);

}
