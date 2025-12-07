#include "br_software_renderer.h"
#include "borka_math.h"
#include <stddef.h>

void software_clear(int *pixels, BrVec2 canvas_dimensions, int color) {
  for (int i = 0; i < canvas_dimensions.x * canvas_dimensions.y; ++i) {
    pixels[i] = color;
  }
}

// TODO: Fix this
void software_draw_filled_triangle(int *pixels, BrVec2 canvas_dimensions,
                                   BrVec2 v0, BrVec2 v1, BrVec2 v2, int color) {
  /*
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
  */
}

void software_draw_rectangle_filled(int *pixels, BrVec2 canvas_dimensions,
                                    BrVec2 position, BrVec2 size, int color) {
  int minX = clamp_int(position.x, 0, canvas_dimensions.x - 1);
  int maxX = clamp_int(position.x + size.x, 0, canvas_dimensions.x - 1);
  int minY = clamp_int(position.y, 0, canvas_dimensions.y - 1);
  int maxY = clamp_int(position.y + size.y, 0, canvas_dimensions.y - 1);

  for (int y = minY; y <= maxY; ++y) {
    int rowOffset = y * canvas_dimensions.y;
    for (int x = minX; x <= maxX; ++x) {
      pixels[rowOffset + x] = color;
    }
  }
}

void software_draw_rectangle_outlined(int *pixels, BrVec2 canvas_dimensions,
                                      BrVec2 position, BrVec2 size, int color) {
  int minX = clamp_int(position.x, 0, canvas_dimensions.x - 1);
  int maxX = clamp_int(position.x + size.x, 0, canvas_dimensions.x - 1);
  int minY = clamp_int(position.y, 0, canvas_dimensions.y - 1);
  int maxY = clamp_int(position.y + size.y, 0, canvas_dimensions.y - 1);

  for (int x = minX; x <= maxX; ++x) {
    pixels[minY * canvas_dimensions.y + x] = color;
    pixels[maxY * canvas_dimensions.y + x] = color;
  }

  for (int y = minY; y <= maxY; ++y) {
    pixels[y * canvas_dimensions.y + minX] = color;
    pixels[y * canvas_dimensions.y + maxX] = color;
  }
}

void software_draw_texture(int *pixels, BrVec2 canvas_dimensions,
                           BrVec2 position, const BrTexture *texture) {
  int startX = clamp_int(-position.x, 0, texture->width);
  int startY = clamp_int(-position.y, 0, texture->height);
  int endX = clamp_int(canvas_dimensions.x - position.x, 0, texture->width);
  int endY = clamp_int(canvas_dimensions.y - position.y, 0, texture->height);

  for (int textureY = startY; textureY < endY; ++textureY) {
    for (int textureX = startX; textureX < endX; ++textureX) {
      int screenX = position.x + textureX;
      int screenY = position.y + textureY;

      if (screenX < 0 || screenX >= canvas_dimensions.x || screenY < 0 ||
          screenY >= canvas_dimensions.y)
        continue;

      int texture_color = texture->pixels[textureY * texture->width + textureX];

      int alpha = (texture_color >> 24) & 0xFF;
      if (alpha == 0)
        continue;

      pixels[screenY * canvas_dimensions.x + screenX] = texture_color;
    }
  }
}
