#include "pch.h"

#include "borka_font.h"
#include "borka_log.h"

bool br_font_init(BrFont *font, BrTexture *atlas, BrVec2 glyph_size,
                  BrVec2 spacing, const char *charset) {
  assert(font && atlas && charset);
  assert(glyph_size.x > 0 && glyph_size.y > 0);

  font->font_atlas = atlas;
  font->glyph_size = glyph_size;
  font->spacing = spacing;
  for (int i = 0; i < BR_FONT_MAX_GLYPHS; i++)
    font->glyph_positions[i] = (BrVec2){-1, -1};

  int cols = atlas->size.x / glyph_size.x;
  int rows = atlas->size.y / glyph_size.y;
  int length = strlen(charset);
  if (length > cols * rows) {
    BR_LOG_ERROR("Font charset has %d characters but the atlas only fits %d",
                 length, cols * rows);
    return false;
  }

  for (int i = 0; i < length; i++) {
    BrVec2 position = {(i % cols) * glyph_size.x, (i / cols) * glyph_size.y};
    font->glyph_positions[(unsigned char)charset[i]] = position;
  }

  return true;
}

int br_font_text_width(const BrFont *font, const char *text) {
  assert(font && text);

  int length = strlen(text);
  if (length == 0)
    return 0;
  return length * (font->glyph_size.x + font->spacing.x) - font->spacing.x;
}
