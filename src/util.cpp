#include "include/util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// the flen-th byte is 0
char* read_file(const char* file, i32* flen, Arena* arena)
{
    FILE* fptr = fopen(file, "rb");
    if (fptr == NULL) {
        printf("Failed to read file: %s\n", file);
        return NULL;
    }
    fseek(fptr, 0, SEEK_END);
    i32 len = ftell(fptr);
    char* buf = NULL;
    if (arena) {
        buf = (char*) push_size(arena, len + 1);
    } else {
        buf = (char*) malloc(len + 1);
    }
    fseek(fptr, 0, SEEK_SET);
    fread(buf, len, 1, fptr);
    buf[len] = 0;
    if (flen)
        *flen = len;
    fclose(fptr);
    return buf;
}


bool prefix(const char* prefix, const char** ptr)
{
    i32 i = 0;
    while (prefix[i]) {
        if (prefix[i] != (*ptr)[i])
            return false;
        ++i;
    }
    (*ptr) += i;
    return true;
}

void skip_whitespaces(const char** ptr)
{
    while (**ptr == ' ') {
        (*ptr)++;
    }
}

void next_line(const char** ptr) 
{
    while (**ptr != '\n' && **ptr != 0) {
        (*ptr)++;
    }
    // praise windows
    while (**ptr == '\n' || **ptr == '\r') {
        (*ptr)++;
    }
}

float read_float(const char** ptr)
{
    float result = atof(*ptr);
    while (**ptr != 0 && **ptr != ' ' && **ptr != '\n') {
        (*ptr)++;
    }
    return result;
}

i32 read_int(const char** ptr)
{
    i32 result = atoi(*ptr);
    while (**ptr != 0 && **ptr != ' ' && **ptr != '\n') {
        (*ptr)++;
    }
    return result;
}


