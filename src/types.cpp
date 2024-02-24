#include "include/types.h"


Str from_c_str(const char* c_str, Arena* arena)
{
    Str res = {};
    const char* ptr = c_str;
    while (*ptr) {
        res.len++;
        ptr++;
    }

    res.ptr = (char*) push_size(arena, sizeof(char) * res.len);

    for (u32 i = 0; i < res.len; ++i) {
        res.ptr[i] = c_str[i];
    }

    return res;
}

bool str_equals(Str a, Str b)
{
    if (a.len != b.len) {
        return false;
    }

    for (u32 i = 0; i < a.len; ++i) {
        if (a.ptr[i] != b.ptr[i]) {
            return false;
        }
    }

    return true;
}
