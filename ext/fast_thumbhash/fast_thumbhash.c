#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct encoded_channel
{
  double dc;
  double *ac;
  int ac_size;
  double scale;
} encoded_channel;

encoded_channel encode_channel(double *channel, u_int8_t nx, u_int8_t ny, u_int8_t w, u_int8_t h)
{
  double dc = 0, scale = 0;
  double *ac = (double *)malloc(nx * ny * sizeof(double));
  double *fx = (double *)malloc(w * sizeof(double));
  int ac_length = 0;

  for (int cy = 0; cy < ny; cy++)
  {
    for (int cx = 0; cx * ny < nx * (ny - cy); cx++)
    {
      double f = 0;

      for (int x = 0; x < w; x++)
      {
        fx[x] = cos(M_PI / w * cx * (x + 0.5));
      }

      for (int y = 0; y < h; y++)
      {
        double fy = cos(M_PI / h * cy * (y + 0.5));
        for (int x = 0; x < w; x++)
        {
          f += channel[x + y * w] * fx[x] * fy;
        }
      }

      f /= w * h;

      if (cx || cy)
      {
        ac[ac_length++] = f;
        scale = fmax(scale, fabsl(f));
      }
      else
      {
        dc = f;
      }
    }
  }

  if (scale)
  {
    for (int i = 0; i < ac_length; i++)
    {
      ac[i] = 0.5 + 0.5 / scale * ac[i];
    }
  }

  free(fx);

  return (encoded_channel){dc, ac, ac_length, scale};
}

void write_varying_factors(double *ac, int ac_size, u_int8_t *thumbhash, u_int8_t *ac_index) {
  for (int i = 0; i < ac_size; i++) {
    u_int8_t index = (*ac_index) >> 1;
    thumbhash[index] |= (u_int8_t) roundf(15.0 * ac[i]) << (((*ac_index)++ & 1) << 2);
  }
}

void rgba_to_thumbhash(u_int8_t w, u_int8_t h, u_int8_t *rgba, u_int8_t *thumbhash)
{
  assert(w <= 100);
  assert(h <= 100);

  // Determine the average color
  double avg_r = 0, avg_g = 0, avg_b = 0, avg_a = 0;

  for (int i = 0, j = 0; i < w * h; i++, j += 4)
  {
    double alpha = (double)rgba[j + 3] / 255.0;
    avg_r += alpha / 255.0 * rgba[j];
    avg_g += alpha / 255.0 * rgba[j + 1];
    avg_b += alpha / 255.0 * rgba[j + 2];
    avg_a += alpha;
  }

  if (avg_a)
  {
    avg_r /= avg_a;
    avg_g /= avg_a;
    avg_b /= avg_a;
  }

  bool has_alpha = avg_a < w * h;
  u_int8_t l_limit = has_alpha ? 5 : 7; // Use fewer luminance bits if there's alpha
  u_int8_t lx = fmax(1, roundf((double)l_limit * w / fmax(w, h)));
  u_int8_t ly = fmax(1, roundf((double)l_limit * h / fmax(w, h)));

  double *l = (double *)malloc(w * h * sizeof(double)); // luminance
  double *p = (double *)malloc(w * h * sizeof(double)); // yellow - blue
  double *q = (double *)malloc(w * h * sizeof(double)); // red - green
  double *a = (double *)malloc(w * h * sizeof(double)); // alpha

  // Convert the image from RGBA to LPQA (composite atop the average color)
  for (int i = 0, j = 0; i < w * h; i++, j += 4)
  {
    double alpha = (double)rgba[j + 3] / 255;
    double r = avg_r * (1.0 - alpha) + alpha / 255.0 * (double)rgba[j];
    double g = avg_g * (1.0 - alpha) + alpha / 255.0 * (double)rgba[j + 1];
    double b = avg_b * (1.0 - alpha) + alpha / 255.0 * (double)rgba[j + 2];
    l[i] = (r + g + b) / 3;
    p[i] = (r + g) / 2 - b;
    q[i] = r - g;
    a[i] = alpha;
  }

  // Encode using the DCT into DC (constant) and normalized AC (varying) terms
  encoded_channel el = encode_channel(l, fmax(3, lx), fmax(3, ly), w, h);
  encoded_channel ep = encode_channel(p, 3, 3, w, h);
  encoded_channel eq = encode_channel(q, 3, 3, w, h);
  encoded_channel ea = has_alpha ? encode_channel(a, 5, 5, w, h) : (encoded_channel){1.0, NULL, 0, 1.0};

  free(l);
  free(p);
  free(q);
  free(a);

  // Write the constants
  bool is_landscape = w > h;
  u_int32_t header24 = (u_int32_t)roundf(63.0 * el.dc) | ((u_int32_t)roundf(31.5 + 31.5 * ep.dc) << 6) | ((u_int32_t)roundf(31.5 + 31.5 * eq.dc) << 12) | ((u_int32_t)roundf(31.0 * el.scale) << 18) | (has_alpha << 23);
  u_int32_t header16 = (is_landscape ? ly : lx) | ((u_int16_t)roundf(63.0 * ep.scale) << 3) | ((u_int16_t)roundf(63.0 * eq.scale) << 9) | (is_landscape << 15);

  thumbhash[0] = (uint8_t)(header24 & 0xff);
  thumbhash[1] = (uint8_t)((header24 >> 8) & 0xff);
  thumbhash[2] = (uint8_t)(header24 >> 16);
  thumbhash[3] = (uint8_t)(header16 & 0xff);
  thumbhash[4] = (uint8_t)(header16 >> 8);

  if (has_alpha) {
    thumbhash[5] = (uint8_t) roundf(15.0 * ea.dc) | ((uint8_t) roundf(15.0 * ea.scale) << 4);
  }

  u_int8_t ac_start = has_alpha ? 6 : 5;
  u_int8_t ac_index = 0;

  write_varying_factors(el.ac, el.ac_size, thumbhash + ac_start, &ac_index);
  write_varying_factors(ep.ac, ep.ac_size, thumbhash + ac_start, &ac_index);
  write_varying_factors(eq.ac, eq.ac_size, thumbhash + ac_start, &ac_index);

  if (has_alpha) {
    write_varying_factors(ea.ac, ea.ac_size, thumbhash + ac_start, &ac_index);
  }

  free(el.ac);
  free(ep.ac);
  free(eq.ac);

  if (has_alpha) {
    free(ea.ac);
  }
}

double *decode_channel(u_int8_t nx, u_int8_t ny, double scale, u_int8_t *hash, u_int8_t ac_start, u_int8_t *ac_index)
{
  double *ac = (double *)malloc(nx * ny * sizeof(double));
  int i = 0;
  for (int cy = 0; cy < ny; cy++)
  {
    for (int cx = cy ? 0 : 1; cx * ny < nx * (ny - cy); cx++)
    {
      double bit = (hash[ac_start + (*ac_index >> 1)] >> (((*ac_index)++ & 1) << 2)) & 15;
      ac[i++] = (bit / 7.5 - 1) * scale;
    }
  }

  return ac;
}

double thumbhash_to_approximate_aspect_ratio(u_int8_t *hash)
{
  u_int8_t has_alpha = (hash[2] & 0x80) != 0;
  u_int8_t l_max = has_alpha ? 5 : 7;
  u_int8_t l_min = hash[3] & 7;
  u_int8_t is_landscape = (hash[4] & 0x80) != 0;
  u_int8_t lx = is_landscape ? l_max : l_min;
  u_int8_t ly = is_landscape ? l_min : l_max;

  return (double)lx / (double)ly;
}

void thumb_size(u_int8_t *hash, u_int8_t max_size, u_int8_t *size)
{
  double ratio = thumbhash_to_approximate_aspect_ratio(hash);

  size[0] = roundf(ratio > 1 ? max_size : max_size * ratio);
  size[1] = roundf(ratio > 1 ? max_size / ratio : max_size);
}

/**
 * Decodes a ThumbHash to an RGBA image. RGB is not be premultiplied by A.
 */
void thumbhash_to_rgba(u_int8_t *hash, u_int8_t w, u_int8_t h, u_int8_t *rgba)
{
  // Read the constants
  u_int32_t header24 = hash[0] | (hash[1] << 8) | (hash[2] << 16);
  u_int32_t header16 = hash[3] | (hash[4] << 8);
  double l_dc = (double)(header24 & 63) / 63;
  double p_dc = (double)((header24 >> 6) & 63) / 31.5 - 1;
  double q_dc = (double)((header24 >> 12) & 63) / 31.5 - 1;
  double l_scale = (double)((header24 >> 18) & 31) / 31;
  bool has_alpha = (header24 >> 23) != 0;
  double p_scale = (double)((header16 >> 3) & 63) / 63;
  double q_scale = (double)((header16 >> 9) & 63) / 63;
  bool is_landscape = (header16 >> 15) != 0;
  u_int8_t lx = fmax(3, is_landscape ? has_alpha ? 5 : 7 : header16 & 7);
  u_int8_t ly = fmax(3, is_landscape ? header16 & 7 : has_alpha ? 5
                                                                : 7);
  double a_dc = (double)has_alpha ? (hash[5] & 15) / 15 : 1;
  double a_scale = (double)(hash[5] >> 4) / 15;

  // Read the varying factors (boost saturation by 1.25x to compensate for quantization)
  u_int8_t ac_start = has_alpha ? 6 : 5;
  u_int8_t ac_index = 0;

  double *l_ac = decode_channel(lx, ly, l_scale, hash, ac_start, &ac_index);
  double *p_ac = decode_channel(3, 3, p_scale * 1.25, hash, ac_start, &ac_index);
  double *q_ac = decode_channel(3, 3, q_scale * 1.25, hash, ac_start, &ac_index);
  double *a_ac = has_alpha ? decode_channel(5, 5, a_scale, hash, ac_start, &ac_index) : NULL;

  // Decode using the DCT into RGB
  double fx[7];
  double fy[7];
  u_int32_t i = 0;

  for (u_int8_t y = 0; y < h; y++)
  {
    for (u_int8_t x = 0; x < w; x++, i += 4)
    {
      double l = l_dc, p = p_dc, q = q_dc, a = a_dc;

      // Precompute the coefficients
      for (int cx = 0, n = fmax(lx, has_alpha ? 5 : 3); cx < n; cx++)
      {
        fx[cx] = cos(M_PI / w * (x + 0.5) * cx);
      }
      for (int cy = 0, n = fmax(ly, has_alpha ? 5 : 3); cy < n; cy++)
      {
        fy[cy] = cos(M_PI / h * (y + 0.5) * cy);
      }

      // Decode L
      double fy2;
      for (int cy = 0, j = 0; cy < ly; cy++)
      {
        fy2 = fy[cy] * 2;
        for (int cx = cy ? 0 : 1; cx * ly < lx * (ly - cy); cx++, j++)
        {
          l += l_ac[j] * fx[cx] * fy2;
        }
      }

      // Decode P and Q
      for (int cy = 0, j = 0; cy < 3; cy++)
      {
        fy2 = fy[cy] * 2;
        for (int cx = cy ? 0 : 1; cx < 3 - cy; cx++, j++)
        {
          double f = fx[cx] * fy2;
          p += p_ac[j] * f;
          q += q_ac[j] * f;
        }
      }

      // Decode A
      if (has_alpha)
      {
        for (int cy = 0, j = 0; cy < 5; cy++)
        {
          fy2 = fy[cy] * 2;
          for (int cx = cy ? 0 : 1; cx < 5 - cy; cx++, j++)
          {
            a += a_ac[j] * fx[cx] * fy2;
          }
        }
      }

      // Convert to RGB
      double b = l - 2.0 / 3.0 * p;
      double r = (3.0 * l - b + q) / 2.0;
      double g = r - q;

      rgba[i] = fmax(0, 255 * fmin(1, r));
      rgba[i + 1] = fmax(0, 255 * fmin(1, g));
      rgba[i + 2] = fmax(0, 255 * fmin(1, b));
      rgba[i + 3] = fmax(0, 255 * fmin(1, a));
    }
  }

  free(l_ac);
  free(p_ac);
  free(q_ac);

  if (has_alpha)
  {
    free(a_ac);
  }
}