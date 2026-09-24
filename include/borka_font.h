#ifndef BORKA_FONT_H
#define BORKA_FONT_H

#include "borka_math.h"
#include "borka_texture.h"

/**
 * @brief Bitmap font structure for text rendering
 *
 * @note All glyphs should be the same size (monospace font)
 * @note The atlas is a single row: A-Z followed by 0-9. Other characters
 * have no glyph and are left blank.
 */
typedef struct {
  BrTexture *font_atlas; /**< Texture atlas containing all glyphs. */
  BrVec2 glyph_size;     /**< Size of each glyph in pixels (width, height). */
  BrVec2 spacing;        /**< Space between glyphs when rendering (horizontal,
                            vertical) in pixels. */
} BrFont;

/**
 * @brief Width in pixels of a text when drawn with br_renderer_draw_text().
 *
 * Every character takes the glyph width plus the horizontal spacing. No
 * spacing is counted after the final character.
 *
 * @param font Font to measure with. Must not be NULL.
 * @param text Text to measure. Must not be NULL.
 * @return The width in pixels.
 */
int br_font_text_width(const BrFont *font, const char *text);

#endif // BORKA_FONT_H
