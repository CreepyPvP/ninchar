#ifndef UTIL_H
#define UTIL_H

#include "include/types.h"
#include "include/arena.h"

char* read_file(const char* file, i32* flen, Arena* arena);


// returns true if ptr starts with prefix. 
// Also removes the prefix string from ptr
bool prefix(const char* prefix, const char** ptr);

void skip_whitespaces(const char** ptr);

void next_line(const char** ptr);

float read_float(const char** ptr);

i32 read_int(const char** ptr);


#endif
