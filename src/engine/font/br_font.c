#include "pch.h"

#include "borka_font.h"

int br_font_text_width(const BrFont *font, const char *text) {
  assert(font && text);

  int length = strlen(text);
  if (length == 0)
    return 0;
  return length * (font->glyph_size.x + font->spacing.x) - font->spacing.x;
}
