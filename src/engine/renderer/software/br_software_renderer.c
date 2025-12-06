#include "br_software_renderer.h"
#include "borka_math.h"
#include <stddef.h>

void software_clear(int *pixels, int width, int height, int color) {
  for (int i = 0; i < width * height; ++i) {
    pixels[i] = color;
  }
}

void software_draw_filled_triangle(int *pixels, int width, int height,
                                   BrVec2 v0, BrVec2 v1, BrVec2 v2, int color) {
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

void software_draw_rectangle_filled(int *pixels, int width, int height, int x,
                                    int y, int rect_width, int rect_height,
                                    int color) {
  int minX = clamp_int(x, 0, width - 1);
  int maxX = clamp_int(x + rect_width, 0, width - 1);
  int minY = clamp_int(y, 0, height - 1);
  int maxY = clamp_int(y + rect_height, 0, height - 1);

  for (int y = minY; y <= maxY; ++y) {
    int rowOffset = y * width;
    for (int x = minX; x <= maxX; ++x) {
      pixels[rowOffset + x] = color;
    }
  }
}

void software_draw_rectangle_outlined(int *pixels, int width, int height, int x,
                                      int y, int rect_width, int rect_height,
                                      int color) {
  int minX = clamp_int(x, 0, width - 1);
  int maxX = clamp_int(x + rect_width, 0, width - 1);
  int minY = clamp_int(y, 0, height - 1);
  int maxY = clamp_int(y + rect_height, 0, height - 1);

  for (int x = minX; x <= maxX; ++x) {
    pixels[minY * width + x] = color;
    pixels[maxY * width + x] = color;
  }

  for (int y = minY; y <= maxY; ++y) {
    pixels[y * width + minX] = color;
    pixels[y * width + maxX] = color;
  }
}

void software_draw_texture(int *pixels, int width, int height, int x, int y,
                           const BrTexture *texture) {
  int startX = clamp_int(-x, 0, texture->width);
  int startY = clamp_int(-y, 0, texture->height);
  int endX = clamp_int(width - x, 0, texture->width);
  int endY = clamp_int(height - y, 0, texture->height);

  for (int textureY = startY; textureY < endY; ++textureY) {
    for (int textureX = startX; textureX < endX; ++textureX) {
      int screenX = x + textureX;
      int screenY = y + textureY;

      if (screenX < 0 || screenX >= width || screenY < 0 || screenY >= height)
        continue;

      int texture_color = texture->pixels[textureY * texture->width + textureX];

      int alpha = (texture_color >> 24) & 0xFF;
      if (alpha == 0)
        continue;

      pixels[screenY * width + screenX] = texture_color;
    }
  }
}
