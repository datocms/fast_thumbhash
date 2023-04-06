#ifndef __FAST_THUMBHASH_H__
#define __FAST_THUMBHASH_H__

#include <stdio.h>

void rgba_to_thumbhash(u_int8_t w, u_int8_t h, u_int8_t *rgba, u_int8_t *thumbhash);

void thumbhash_to_rgba(u_int8_t *hash, u_int8_t w, u_int8_t h, u_int8_t *rgba);
void thumb_size(u_int8_t *hash, u_int8_t max_size, u_int8_t *size);

#endif