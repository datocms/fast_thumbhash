#ifndef __FAST_THUMBHASH_H__
#define __FAST_THUMBHASH_H__

#include <stdint.h>

enum FillMode
{
  SOLID = 0,
  BLUR = 1,
  CLAMP = 2,
};

uint8_t rgba_to_thumbhash(
    uint8_t w,
    uint8_t h,
    uint8_t *rgba,
    uint8_t *thumbhash);

void thumb_size(
    uint8_t *hash,
    uint8_t max_size,
    uint8_t *size);

void thumbhash_to_rgba(
    uint8_t *hash,
    uint8_t w,
    uint8_t h,
    enum FillMode fill_mode,
    uint8_t *fill_color,
    double *homogeneous_transform,
    int saturation,
    uint8_t *rgba);

#endif