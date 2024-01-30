#include "include/renderer.h"

#include <stdio.h>

CommandBuffer command_buffer(u32 entry_cap, u8* entry_buffer, 
                             u32 vert_cap, Vertex* vert_buffer, 
                             u32 index_cap, u32* index_buffer,
                             Mat4 proj)
{
    CommandBuffer buffer;
    buffer.entry_buffer = entry_buffer;
    buffer.entry_cap = entry_cap;
    buffer.entry_size = 0;
    buffer.vert_buffer = vert_buffer;
    buffer.vert_cap = vert_cap;
    buffer.vert_count = 0;
    buffer.index_buffer = index_buffer;
    buffer.index_cap = index_cap;
    buffer.index_count = 0;
    buffer.proj = proj;
    return buffer;
}

u8* push_entry(CommandBuffer* buffer, u32 size)
{
    if (buffer->entry_size + size > buffer->entry_cap) {
        printf("Command buffer size exceeded\n");
        return NULL;
    }

    u8* entry = buffer->entry_buffer + buffer->entry_size;
    buffer->entry_size += size;
    return entry;
}

void push_clear(CommandBuffer* buffer, V3 color)
{
    CommandEntry_Clear* clear = (CommandEntry_Clear*) push_entry(buffer, sizeof(CommandEntry_Clear));
    if (!clear) {
        return;
    }

    clear->header.type = EntryType_Clear;
    clear->color = color;
}

void push_quad(CommandBuffer* buffer, V2 down_left, V2 up_right, V3 color)
{
    CommandEntry_Draw* draw = (CommandEntry_Draw*) push_entry(buffer, sizeof(CommandEntry_Draw));
    if (!draw) {
        return;
    }
    if (buffer->vert_count + 4 > buffer->vert_cap) {
        printf("Warning: Vertex buffer size exceede\n");
        return;
    }
    if (buffer->index_count + 6 > buffer->index_cap) {
        printf("Warning: Index buffer size exceede\n");
        return;
    }

    draw->header.type = EntryType_Draw;
    draw->index_offset = buffer->index_count;
    draw->index_count = 6;

    u32 vcurr = buffer->vert_count;
    buffer->vert_buffer[vcurr + 0].pos.x = down_left.x;
    buffer->vert_buffer[vcurr + 0].pos.y = down_left.y;
    buffer->vert_buffer[vcurr + 0].pos.z = 0;
    buffer->vert_buffer[vcurr + 0].color = color;

    buffer->vert_buffer[vcurr + 1].pos.x = down_left.x;
    buffer->vert_buffer[vcurr + 1].pos.y = up_right.y;
    buffer->vert_buffer[vcurr + 1].pos.z = 0;
    buffer->vert_buffer[vcurr + 1].color = color;

    buffer->vert_buffer[vcurr + 2].pos.x = up_right.x;
    buffer->vert_buffer[vcurr + 2].pos.y = up_right.y;
    buffer->vert_buffer[vcurr + 2].pos.z = 0;
    buffer->vert_buffer[vcurr + 2].color = color;

    buffer->vert_buffer[vcurr + 3].pos.x = up_right.x;
    buffer->vert_buffer[vcurr + 3].pos.y = down_left.y;
    buffer->vert_buffer[vcurr + 3].pos.z = 0;
    buffer->vert_buffer[vcurr + 3].color = color;

    u32 icurr = buffer->index_count;
    buffer->index_buffer[icurr + 0] = 0;
    buffer->index_buffer[icurr + 1] = 1;
    buffer->index_buffer[icurr + 2] = 2;
    buffer->index_buffer[icurr + 3] = 0;
    buffer->index_buffer[icurr + 4] = 2;
    buffer->index_buffer[icurr + 5] = 3;
    
    buffer->index_count += 6;
    buffer->vert_count += 4;
}
