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
