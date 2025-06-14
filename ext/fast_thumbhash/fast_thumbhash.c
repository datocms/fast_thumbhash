#include <fast_thumbhash.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct encoded_channel
{
  double dc;
  double *ac;
  int ac_size;
  double scale;
} encoded_channel;

encoded_channel encode_channel(double *channel, uint8_t nx, uint8_t ny, uint8_t w, uint8_t h)
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

uint8_t write_varying_factors(double *ac, int ac_size, uint8_t *thumbhash, uint8_t *ac_index)
{
  uint8_t index;

  for (int i = 0; i < ac_size; i++)
  {
    index = (*ac_index) >> 1;
    thumbhash[index] |= (uint8_t)roundf(15.0 * ac[i]) << (((*ac_index)++ & 1) << 2);
  }

  return index;
}

uint8_t rgba_to_thumbhash(uint8_t w, uint8_t h, uint8_t *rgba, uint8_t *thumbhash)
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
  uint8_t l_limit = has_alpha ? 5 : 7; // Use fewer luminance bits if there's alpha
  uint8_t lx = fmax(1, roundf((double)l_limit * w / fmax(w, h)));
  uint8_t ly = fmax(1, roundf((double)l_limit * h / fmax(w, h)));

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
  uint32_t header24 = (uint32_t)roundf(63.0 * el.dc) | ((uint32_t)roundf(31.5 + 31.5 * ep.dc) << 6) | ((uint32_t)roundf(31.5 + 31.5 * eq.dc) << 12) | ((uint32_t)roundf(31.0 * el.scale) << 18) | (has_alpha << 23);
  uint32_t header16 = (is_landscape ? ly : lx) | ((uint16_t)roundf(63.0 * ep.scale) << 3) | ((uint16_t)roundf(63.0 * eq.scale) << 9) | (is_landscape << 15);

  thumbhash[0] = (uint8_t)(header24 & 0xff);
  thumbhash[1] = (uint8_t)((header24 >> 8) & 0xff);
  thumbhash[2] = (uint8_t)(header24 >> 16);
  thumbhash[3] = (uint8_t)(header16 & 0xff);
  thumbhash[4] = (uint8_t)(header16 >> 8);

  if (has_alpha)
  {
    thumbhash[5] = (uint8_t)roundf(15.0 * ea.dc) | ((uint8_t)roundf(15.0 * ea.scale) << 4);
  }

  uint8_t ac_start = has_alpha ? 6 : 5;
  uint8_t ac_index = 0;

  uint8_t max_written_indexes[] = {0, 0, 0, 0};

  max_written_indexes[0] = write_varying_factors(el.ac, el.ac_size, thumbhash + ac_start, &ac_index);
  max_written_indexes[1] = write_varying_factors(ep.ac, ep.ac_size, thumbhash + ac_start, &ac_index);
  max_written_indexes[2] = write_varying_factors(eq.ac, eq.ac_size, thumbhash + ac_start, &ac_index);

  if (has_alpha)
  {
    max_written_indexes[3] = write_varying_factors(ea.ac, ea.ac_size, thumbhash + ac_start, &ac_index);
  }

  free(el.ac);
  free(ep.ac);
  free(eq.ac);

  if (has_alpha)
  {
    free(ea.ac);
  }

  // store the largest number at max_written_indexes[0]
  for (int i = 1; i < 4; ++i)
  {
    if (max_written_indexes[0] < max_written_indexes[i])
    {
      max_written_indexes[0] = max_written_indexes[i];
    }
  }

  return max_written_indexes[0] + ac_start + 1;
}

double *decode_channel(uint8_t nx, uint8_t ny, double scale, uint8_t *hash, uint8_t ac_start, uint8_t *ac_index)
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

double thumbhash_to_approximate_aspect_ratio(uint8_t *hash)
{
  uint8_t has_alpha = (hash[2] & 0x80) != 0;
  uint8_t l_max = has_alpha ? 5 : 7;
  uint8_t l_min = hash[3] & 7;
  uint8_t is_landscape = (hash[4] & 0x80) != 0;
  uint8_t lx = is_landscape ? l_max : l_min;
  uint8_t ly = is_landscape ? l_min : l_max;

  return (double)lx / (double)ly;
}

void thumb_size(uint8_t *hash, uint8_t max_size, uint8_t *size)
{
  double ratio = thumbhash_to_approximate_aspect_ratio(hash);

  size[0] = roundf(ratio > 1 ? max_size : max_size * ratio);
  size[1] = roundf(ratio > 1 ? max_size / ratio : max_size);
}

void rgb2hsv(uint8_t *rgb, float *hsv)
{
  float min, max, delta;

  float r = rgb[0] / 255.0;
  float g = rgb[1] / 255.0;
  float b = rgb[2] / 255.0;

  min = r < g ? r : g;
  min = min < b ? min : b;

  max = r > g ? r : g;
  max = max > b ? max : b;

  hsv[2] = max;

  delta = max - min;

  if (delta < 0.00001)
  {
    hsv[1] = 0;
    hsv[0] = 0; // undefined, maybe nan?
    return;
  }
  if (max > 0.0)
  {                         // NOTE: if Max is == 0, this divide would cause a crash
    hsv[1] = (delta / max); // s
  }
  else
  {
    // if max is 0, then r = g = b = 0
    // s = 0, h is undefined
    hsv[1] = 0.0;
    hsv[0] = NAN; // its now undefined
    return;
  }

  if (r >= max)               // > is bogus, just keeps compilor happy
    hsv[0] = (g - b) / delta; // between yellow & magenta
  else if (g >= max)
    hsv[0] = 2.0 + (b - r) / delta; // between cyan & yellow
  else
    hsv[0] = 4.0 + (r - g) / delta; // between magenta & cyan

  hsv[0] *= 60.0; // degrees

  if (hsv[0] < 0.0)
    hsv[0] += 360.0;
}

void hsv2rgb(float *hsv, uint8_t *rgb)
{
  float hh, p, q, t, ff;
  long i;

  if (hsv[1] <= 0.0)
  { // < is bogus, just shuts up warnings
    rgb[0] = hsv[2] * 255.0;
    rgb[1] = hsv[2] * 255.0;
    rgb[2] = hsv[2] * 255.0;
    return;
  }

  hh = hsv[0];

  if (hh >= 360.0)
    hh = 0.0;

  hh /= 60.0;

  i = (long)hh;

  ff = hh - i;
  p = hsv[2] * (1.0 - hsv[1]);
  q = hsv[2] * (1.0 - (hsv[1] * ff));
  t = hsv[2] * (1.0 - (hsv[1] * (1.0 - ff)));

  switch (i)
  {
  case 0:
    rgb[0] = hsv[2] * 255.0;
    rgb[1] = t * 255.0;
    rgb[2] = p * 255.0;
    return;
  case 1:
    rgb[0] = q * 255.0;
    rgb[1] = hsv[2] * 255.0;
    rgb[2] = p * 255.0;
    return;
  case 2:
    rgb[0] = p * 255.0;
    rgb[1] = hsv[2] * 255.0;
    rgb[2] = t * 255.0;
    return;

  case 3:
    rgb[0] = p * 255.0;
    rgb[1] = q * 255.0;
    rgb[2] = hsv[2] * 255.0;
    return;
  case 4:
    rgb[0] = t * 255.0;
    rgb[1] = p * 255.0;
    rgb[2] = hsv[2] * 255.0;
    return;
  case 5:
  default:
    rgb[0] = hsv[2] * 255.0;
    rgb[1] = p * 255.0;
    rgb[2] = q * 255.0;
    return;
  }
}

/**
 * Decodes a ThumbHash to an RGBA image. RGB is not be premultiplied by A.
 */
void thumbhash_to_rgba(
    uint8_t *hash,
    uint8_t w,
    uint8_t h,
    enum FillMode fill_mode,
    uint8_t *fill_color,
    double *homogeneous_transform,
    int saturation,
    uint8_t *rgba)
{
  // Read the constants
  uint32_t header24 = hash[0] | (hash[1] << 8) | (hash[2] << 16);
  uint32_t header16 = hash[3] | (hash[4] << 8);
  double l_dc = (double)(header24 & 63) / 63;
  double p_dc = (double)((header24 >> 6) & 63) / 31.5 - 1;
  double q_dc = (double)((header24 >> 12) & 63) / 31.5 - 1;
  double l_scale = (double)((header24 >> 18) & 31) / 31;
  bool has_alpha = (header24 >> 23) != 0;
  double p_scale = (double)((header16 >> 3) & 63) / 63;
  double q_scale = (double)((header16 >> 9) & 63) / 63;
  bool is_landscape = (header16 >> 15) != 0;
  uint8_t lx = fmax(3, is_landscape ? has_alpha ? 5 : 7 : header16 & 7);
  uint8_t ly = fmax(3, is_landscape ? header16 & 7 : has_alpha ? 5
                                                               : 7);
  double a_dc = (double)has_alpha ? (hash[5] & 15) / 15 : 1;
  double a_scale = (double)(hash[5] >> 4) / 15;

  // Read the varying factors (boost saturation by 1.25x to compensate for quantization)
  uint8_t ac_start = has_alpha ? 6 : 5;
  uint8_t ac_index = 0;

  double *l_ac = decode_channel(lx, ly, l_scale, hash, ac_start, &ac_index);
  double *p_ac = decode_channel(3, 3, p_scale * 1.25, hash, ac_start, &ac_index);
  double *q_ac = decode_channel(3, 3, q_scale * 1.25, hash, ac_start, &ac_index);
  double *a_ac = has_alpha ? decode_channel(5, 5, a_scale, hash, ac_start, &ac_index) : NULL;

  // Decode using the DCT into RGB
  double fx[7];
  double fy[7];
  uint32_t i = 0;

  for (uint8_t ry = 0; ry < h; ry++)
  {
    for (uint8_t rx = 0; rx < w; rx++, i += 4)
    {

      double px = ((double)rx + 0.5) / w;
      double py = ((double)ry + 0.5) / h;

      double x = px;
      double y = py;

      if (homogeneous_transform)
      {
        x = homogeneous_transform[0] * px + homogeneous_transform[1] * py + homogeneous_transform[2];
        y = homogeneous_transform[3] * px + homogeneous_transform[4] * py + homogeneous_transform[5];
      }

      double r, g, b, a;

      if (fill_mode == CLAMP || fill_mode == BLUR)
      {
        if (x < 0)
        {
          x = 0;
        }
        else if (x >= 1)
        {
          x = 1;
        }
        else if (y < 0)
        {
          y = 0;
        }
        else if (y >= 1)
        {
          y = 1;
        }
      }

      bool inside_image = x >= 0 && x <= 1.0 && y >= 0 && y <= 1.0;

      if (inside_image)
      {
        double l = l_dc, p = p_dc, q = q_dc;
        a = a_dc;

        // Precompute the coefficients
        for (int cx = 0, n = fmax(lx, has_alpha ? 5 : 3); cx < n; cx++)
        {
          fx[cx] = cos(M_PI * x * cx);
        }
        for (int cy = 0, n = fmax(ly, has_alpha ? 5 : 3); cy < n; cy++)
        {
          fy[cy] = cos(M_PI * y * cy);
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
        b = l - 2.0 / 3.0 * p;
        r = (3.0 * l - b + q) / 2.0;
        g = r - q;
      }
      else
      {
        r = 255;
        g = 255;
        b = 255;
        a = 0;
      }

      uint_fast8_t top[4] = {
          fmax(0, 255 * fmin(1, r)),
          fmax(0, 255 * fmin(1, g)),
          fmax(0, 255 * fmin(1, b)),
          fmax(0, 255 * fmin(1, a))};

      if (fill_color && fill_color[3] > 0)
      {
        double top_a = (double)top[3] / 255.0;
        double fill_color_a = (double)fill_color[3] / 255.0;
        double inverse_top_a = 1.0 - top_a;
        double sum_a = top_a + fill_color_a * inverse_top_a;

        // Alpha compositing (top over fill_color)
        rgba[i] = roundf(((double)top[0] * top_a + (double)fill_color[0] * fill_color_a * inverse_top_a) / sum_a);
        rgba[i + 1] = roundf(((double)top[1] * top_a + (double)fill_color[1] * fill_color_a * inverse_top_a) / sum_a);
        rgba[i + 2] = roundf(((double)top[2] * top_a + (double)fill_color[2] * fill_color_a * inverse_top_a) / sum_a);
        rgba[i + 3] = roundf(sum_a * 255.0);
      }
      else
      {
        rgba[i] = top[0];
        rgba[i + 1] = top[1];
        rgba[i + 2] = top[2];
        rgba[i + 3] = top[3];
      }

      if (saturation)
      {
        float hsv[3] = {0};
        rgb2hsv(rgba + i, hsv);
        float mult = ((float)saturation + 100.0f) / 200.0f * 1.4f;
        hsv[1] = fminf(fmaxf(hsv[1] * mult, 0), 1.0f);
        hsv2rgb(hsv, rgba + i);
      }
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
