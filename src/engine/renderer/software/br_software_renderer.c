#include "br_software_renderer.h"
#include "../engine/logger/br_logger.h"
#include "../engine/math/br_math.h"
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
  int minX = min_int(min_int(v0.x, v1.x), v2.x);
  int maxX = max_int(max_int(v0.x, v1.x), v2.x);
  int minY = min_int(min_int(v0.y, v1.y), v2.y);
  int maxY = max_int(max_int(v0.y, v1.y), v2.y);

  // Edge vectors (CCW)
  BrVec2 e0 = br_vec2_sub(v1, v0);
  BrVec2 e1 = br_vec2_sub(v2, v1);
  BrVec2 e2 = br_vec2_sub(v0, v2);

  for (int y = minY; y <= maxY; ++y) {
    for (int x = minX; x <= maxX; ++x) {
      BrVec2 p = {x, y};
      int c0 = br_vec2_cross(e0, br_vec2_sub(p, v0));
      if (c0 >= 0) {
        int c1 = br_vec2_cross(e1, br_vec2_sub(p, v1));
        if (c1 >= 0) {
          int c2 = br_vec2_cross(e2, br_vec2_sub(p, v2));
          if (c2 >= 0) {
            pixels[y * width + x] = color;
          }
        }
      }
    }
  }
}

void software_draw_quad() {
  // TODO
}
