#include "br_software_renderer.h"
#include "borka_log.h"
#include "borka_math.h"
#include <stddef.h>

void software_clear(int *pixels, int width, int height, int color) {
  if (!pixels) {
    BR_LOG_WARN("Could not clear renderer: Pixel buffer is NULL");
    return;
  }
  for (int i = 0; i < width * height; ++i) {
    pixels[i] = color;
  }
}

void software_draw_triangle(int *pixels, int width, int height, BrVec2 v0,
                            BrVec2 v1, BrVec2 v2, int color) {
  if (!pixels) {
    BR_LOG_WARN("Skipping draw: Pixel buffer is NULL");
    return;
  }
  int minX = clamp_int(min_int(min_int(v0.x, v1.x), v2.x), 0, width - 1);
  int maxX = clamp_int(max_int(max_int(v0.x, v1.x), v2.x), 0, width - 1);
  int minY = clamp_int(min_int(min_int(v0.y, v1.y), v2.y), 0, height - 1);
  int maxY = clamp_int(max_int(max_int(v0.y, v1.y), v2.y), 0, height - 1);

  // Edge vectors (CCW)
  BrVec2 e0 = br_vec2_sub(v1, v0);
  BrVec2 e1 = br_vec2_sub(v2, v1);
  BrVec2 e2 = br_vec2_sub(v0, v2);

  for (int y = minY; y <= maxY; ++y) {
    for (int x = minX; x <= maxX; ++x) {
      BrVec2 p = {x, y};

      int c0 = br_vec2_cross(e0, br_vec2_sub(p, v0));
      if (c0 < 0)
        continue;

      int c1 = br_vec2_cross(e1, br_vec2_sub(p, v1));
      if (c1 < 0)
        continue;

      int c2 = br_vec2_cross(e2, br_vec2_sub(p, v2));
      if (c2 < 0)
        continue;

      pixels[y * width + x] = color;
    }
  }
}

void software_draw_quad(int *pixels, int width, int height, BrVec2 v0,
                        BrVec2 v1, BrVec2 v2, BrVec2 v3, int color) {
  if (!pixels) {
    BR_LOG_WARN("Skipping draw: Pixel buffer is NULL");
    return;
  }
  int minX = clamp_int(min_int(min_int(min_int(v0.x, v1.x), v2.x), v3.x), 0,
                       width - 1);
  int maxX = clamp_int(max_int(max_int(max_int(v0.x, v1.x), v2.x), v3.x), 0,
                       width - 1);
  int minY = clamp_int(min_int(min_int(min_int(v0.y, v1.y), v2.y), v3.y), 0,
                       height - 1);
  int maxY = clamp_int(max_int(max_int(max_int(v0.y, v1.y), v2.y), v3.y), 0,
                       height - 1);

  for (int y = minY; y <= maxY; ++y) {
    int rowOffset = y * width;
    for (int x = minX; x <= maxX; ++x) {
      pixels[rowOffset + x] = color;
    }
  }
}
