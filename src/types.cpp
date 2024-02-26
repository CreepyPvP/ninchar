#include "include/types.h"


Str str_with_cap(u16 cap, Arena* arena)
{
    Str res = {};
    res.cap = cap;
    res.ptr = (char*) push_size(arena, sizeof(char) * res.cap);
    return res;
}

Str from_c_str(const char* c_str, Arena* arena)
{
    Str res = {};
    const char* ptr = c_str;
    while (*ptr) {
        res.cap++;
        ptr++;
    }

    res.len = res.cap;
    res.ptr = (char*) push_size(arena, sizeof(char) * res.cap);

    for (u32 i = 0; i < res.len; ++i) {
        res.ptr[i] = c_str[i];
    }

    return res;
}

Str str_cpy(Str* str, Arena* arena)
{
    Str res = *str;
    res.ptr = (char*) push_size(arena, sizeof(char) * res.cap);
    for (u32 i = 0; i < str->len; ++i) {
        res.ptr[i] = str->ptr[i];
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

void append_line(Str* str, const char* line)
{
    while (*line) {
        assert(str->len < str->cap);
        str->ptr[str->len] = *line;
        ++str->len;
        ++line;
    }

    assert(str->len < str->cap);
    str->ptr[str->len] = '\n';
    ++str->len;
}

void append_null(Str* str)
{
    assert(str->len < str->cap);
    str->ptr[str->len] = 0;
    ++str->len;
}

void print_str(Str* str)
{
    char arr[1024];
    assert(str->len < 1024);
    for (u32 i = 0; i < str->len; ++i) {
        arr[i] = str->ptr[i];
    }
    arr[str->len] = 0;

    printf("%s\n", arr);
}
