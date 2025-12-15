#ifndef BORKA_FONT_H
#define BORKA_FONT_H

#include "borka_math.h"
#include "borka_texture.h"

typedef struct {
  BrTexture *font_atlas;
  BrVec2 glyph_size;
  BrVec2 spacing;
} BrFont;

#endif // BORKA_FONT_H
