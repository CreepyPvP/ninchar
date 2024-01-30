#ifndef UTIL_H
#define UTIL_H

#include "include/types.h"
#include "include/arena.h"

char* read_file(const char* file, i32* flen, Arena* arena);

#endif
