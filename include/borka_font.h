#ifndef BORKA_FONT_H
#define BORKA_FONT_H

#include "borka_math.h"
#include "borka_texture.h"

/**
 * @brief Bitmap font structure for text rendering
 *
 * @note All glyphs should be the same size (monospace font)
 */
typedef struct {
  BrTexture *font_atlas; /**< Texture atlas containing all glyphs. */
  BrVec2 glyph_size;     /**< Size of each glyph in pixels (width, height). */
  BrVec2 spacing;        /**< Space between glyphs when rendering (horizontal,
                            vertical) in pixels. */
} BrFont;

#endif // BORKA_FONT_H
