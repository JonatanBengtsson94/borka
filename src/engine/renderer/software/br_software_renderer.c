#include "br_software_renderer.h"
#include "borka_log.h"
#include "borka_math.h"
#include "borka_texture.h"
#include <assert.h>

static void blit(int *pixels, BrVec2 canvas_dim, BrVec2 dst_pos,
                 const BrTexture *src, BrVec2 src_rect_pos,
                 BrVec2 src_rect_size) {
  int tex_stride = src->size.x;

  int startX = clamp_int(-dst_pos.x, 0, src_rect_size.x);
  int startY = clamp_int(-dst_pos.y, 0, src_rect_size.y);
  int endX = clamp_int(canvas_dim.x - dst_pos.x, 0, src_rect_size.x);
  int endY = clamp_int(canvas_dim.y - dst_pos.y, 0, src_rect_size.y);

  for (int y = startY; y < endY; ++y) {
    int screenY = dst_pos.y + y;
    int dst_offset = screenY * canvas_dim.x + dst_pos.x;
    int src_offset = (src_rect_pos.y + y) * tex_stride + src_rect_pos.x;

    for (int x = startX; x < endX; ++x) {
      int color = src->pixels[src_offset + x];
      if (((color >> 24) & 0xFF) == 0)
        continue;

      pixels[dst_offset + x] = color;
    }
  }
}

void software_clear(int *pixels, BrVec2 canvas_dimensions, int color) {
  assert(pixels);

  for (int i = 0; i < canvas_dimensions.x * canvas_dimensions.y; ++i) {
    pixels[i] = color;
  }
}

void software_draw_rectangle_filled(int *pixels, BrVec2 canvas_dimensions,
                                    BrVec2 position, BrVec2 size, int color) {
  assert(pixels);

  int minX = clamp_int(position.x, 0, canvas_dimensions.x - 1);
  int maxX = clamp_int(position.x + size.x, 0, canvas_dimensions.x - 1);
  int minY = clamp_int(position.y, 0, canvas_dimensions.y - 1);
  int maxY = clamp_int(position.y + size.y, 0, canvas_dimensions.y - 1);

  for (int y = minY; y <= maxY; ++y) {
    int rowOffset = y * canvas_dimensions.x;
    for (int x = minX; x <= maxX; ++x) {
      pixels[rowOffset + x] = color;
    }
  }
}

void software_draw_rectangle_outlined(int *pixels, BrVec2 canvas_dimensions,
                                      BrVec2 position, BrVec2 size, int color) {
  assert(pixels);

  int minX = clamp_int(position.x, 0, canvas_dimensions.x - 1);
  int maxX = clamp_int(position.x + size.x, 0, canvas_dimensions.x - 1);
  int minY = clamp_int(position.y, 0, canvas_dimensions.y - 1);
  int maxY = clamp_int(position.y + size.y, 0, canvas_dimensions.y - 1);

  for (int x = minX; x <= maxX; ++x) {
    pixels[minY * canvas_dimensions.x + x] = color;
    pixels[maxY * canvas_dimensions.x + x] = color;
  }

  for (int y = minY; y <= maxY; ++y) {
    pixels[y * canvas_dimensions.x + minX] = color;
    pixels[y * canvas_dimensions.x + maxX] = color;
  }
}

void software_draw_texture(int *pixels, BrVec2 canvas_dimensions,
                           BrVec2 position, const BrTexture *texture) {
  assert(pixels && texture && texture->pixels);

  BrVec2 src_pos = {0, 0};
  blit(pixels, canvas_dimensions, position, texture, src_pos, texture->size);
}

void software_draw_texture_region(int *pixels, BrVec2 canvas_dimensions,
                                  BrVec2 position, BrTextureRegion region) {
  assert(pixels && region.texture && region.texture->pixels);
  blit(pixels, canvas_dimensions, position, region.texture, region.position,
       region.size);
}

void software_draw_text(int *pixels, BrVec2 canvas_dimensions, BrVec2 position,
                        const BrFont *font, const char *text) {
  assert(pixels && font && text && font->font_atlas);

  int atlas_w = font->font_atlas->size.x;
  int glyph_w = font->glyph_size.x;
  int glyph_h = font->glyph_size.y;
  int cols = atlas_w / glyph_w;

  BrVec2 cursor = position;

  for (int i = 0; text[i]; i++) {
    // The atlas holds A-Z followed by 0-9. Anything else has no glyph and
    // is left blank rather than indexing outside the atlas.
    char c = text[i];
    int index = -1;
    if (c >= 'A' && c <= 'Z')
      index = c - 'A';
    else if (c >= '0' && c <= '9')
      index = 26 + (c - '0');

    if (index >= 0) {
      BrVec2 src_pos;
      src_pos.x = (index % cols) * glyph_w;
      src_pos.y = (index / cols) * glyph_h;

      blit(pixels, canvas_dimensions, cursor, font->font_atlas, src_pos,
           font->glyph_size);
    }

    // Every character, blank or not, takes the same space (monospace).
    cursor.x += glyph_w;
    cursor.x += font->spacing.x;
  }
  return;
}
