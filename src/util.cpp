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



#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

bool hit_bounding_box(V3 pos, V3 dir, V3 box_pos, V3 box_r, V3* hit, float* t)
{
    V3 min_b = v3(box_pos.x - box_r.x, box_pos.y - box_r.y, box_pos.z - box_pos.z);
    V3 max_b = v3(box_pos.x + box_r.x, box_pos.y + box_r.y, box_pos.z + box_pos.z);

	bool inside = true;
	u8 quadrant[3];
	i32 which_plane;
	V3 max_t;
	V3 candidate_plane;

	for (i32 i = 0; i < 3; ++i)
		if (pos.v[i] < min_b.v[i]) {
			quadrant[i] = LEFT;
			candidate_plane.v[i] = min_b.v[i];
			inside = false;
		} else if (pos.v[i] > max_b.v[i]) {
			quadrant[i] = RIGHT;
			candidate_plane.v[i] = max_b.v[i];
			inside = false;
		} else	{
			quadrant[i] = MIDDLE;
		}

	if (inside)	{
		*hit = pos;
		return true;
	}


	/* Calculate T distances to candidate planes */
	for (i32 i = 0; i < 3; ++i) {
        if (quadrant[i] != MIDDLE && dir.v[i] != 0.) {
            max_t.v[i] = (candidate_plane.v[i] - pos.v[i]) / dir.v[i];
        } else {
            max_t.v[i] = -1.;
        }
    }

	/* Get largest of the maxT's for final choice of intersection */
	which_plane = 0;
	for (i32 i = 1; i < 3; ++i) {
		if (max_t.v[which_plane] < max_t.v[i]) {
			which_plane = i;
        }
    }
    *t = max_t.v[which_plane];

	/* Check final candidate actually inside box */
	if (max_t.v[which_plane] < 0.) return false;
	for (i32 i = 0; i < 3; ++i) {
		if (which_plane != i) {
			hit->v[i] = pos.v[i] + max_t.v[which_plane] * dir.v[i];
			if (hit->v[i] < min_b.v[i] || hit->v[i] > max_b.v[i]) {
				return false;
            }
		} else {
			hit->v[i] = candidate_plane.v[i];
		}
    }

	return true;
}	