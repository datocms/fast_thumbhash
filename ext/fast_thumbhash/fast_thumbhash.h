#ifndef __FAST_THUMBHASH_H__
#define __FAST_THUMBHASH_H__

#include <stdio.h>
#include <stdint.h>

enum FillMode {
  NO_FILL = 0,
  SOLID = 1,
  BLUR = 2,
};

void rgba_to_thumbhash(
  u_int8_t w,
  u_int8_t h,
  u_int8_t *rgba,
  u_int8_t *thumbhash
);

void thumb_size(
  u_int8_t *hash,
  u_int8_t max_size,
  u_int8_t *size
);

void thumbhash_to_rgba(
  u_int8_t *hash,
  u_int8_t w,
  u_int8_t h,
  enum FillMode fill_mode,
  uint8_t *fill_color,
  double *homogeneous_transform,
  int saturation,
  u_int8_t *rgba
);

#endif