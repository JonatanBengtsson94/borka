#ifndef BORKA_FONT_H
#define BORKA_FONT_H

#include "borka_math.h"
#include "borka_texture.h"
#include <stdbool.h>

#define BR_FONT_MAX_GLYPHS 256

/**
 * @brief Bitmap font structure for text rendering
 *
 * @note All glyphs should be the same size (monospace font)
 * @note Set up with br_font_init(), which resolves every glyph's atlas
 * position once so drawing needs no lookups.
 */
typedef struct {
  BrTexture *font_atlas; /**< Texture atlas containing all glyphs. Not owned
                            by the font. */
  BrVec2 glyph_size;     /**< Size of each glyph in pixels (width, height). */
  BrVec2 spacing;        /**< Space between glyphs when rendering (horizontal,
                            vertical) in pixels. */
  BrVec2 glyph_positions[BR_FONT_MAX_GLYPHS]; /**< Atlas position of each
                            character's glyph, {-1, -1} if it has none. */
} BrFont;

/**
 * @brief Sets up a font from an atlas of equally sized glyphs.
 *
 * Glyphs are laid out in the atlas left to right, row by row, in the order
 * given by charset, packed with no padding between them. Characters not in
 * charset are left blank when drawn. Any part of the atlas that does not fill
 * a whole glyph at the right or bottom edge is ignored.
 *
 * @param font Font to set up. Must not be NULL.
 * @param atlas Texture holding the glyphs. Must not be NULL. The font only
 * borrows it, so it has to outlive the font.
 * @param glyph_size Size of each glyph in pixels.
 * @param spacing Space between glyphs when rendering, in pixels.
 * @param charset Characters in atlas order, e.g. "ABCDEFGHIJKLMNOPQRSTUVWXYZ".
 * Only read during this call. Must not be NULL.
 * @return true on success, false if the charset has more characters than
 * the atlas has glyphs. The atlas is stored in the font even on failure, so
 * a caller can still free it through font->font_atlas.
 */
bool br_font_init(BrFont *font, BrTexture *atlas, BrVec2 glyph_size,
                  BrVec2 spacing, const char *charset);

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
