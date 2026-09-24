#ifndef BORKA_RENDER_H
#define BORKA_RENDER_H

#include "borka_font.h"
#include "borka_math.h"
#include "borka_texture.h"

/**
 * @brief Opaque renderer handle.
 *
 * Draw calls are not drawn straight away. They are recorded with a layer and
 * drawn when br_renderer_present() is called: lower layers first, and draws
 * in the same layer in the order they were made. The order of the calls
 * themselves therefore does not matter across layers.
 *
 * Textures passed to draw calls, including a font's atlas, are only read at
 * present, so they must stay alive until then. Text and fonts themselves are
 * only read during the call, so the text may be a temporary.
 */
typedef struct BrRenderer BrRenderer;

/**
 * @brief Starts a new frame, cleared to the specified color.
 *
 * Discards anything drawn since the last present, and clears the frame to the
 * color before any draws are made at present.
 *
 * @param renderer The renderer to clear. Must not be NULL.
 * @param color 32-bit ARGB color value (0xAARRGGBB format).
 */
void br_renderer_clear(BrRenderer *renderer, int color);

/**
 * @brief Draws a filled rectangle.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param layer Draw order, lower layers are drawn first.
 * @param position Position of upper-left corner of the rectangle.
 * @param size Size of the rectangle in pixels.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Only supports axis-aligned rectangles.
 * @note Nothing is drawn until br_renderer_present() is called.
 */
void br_renderer_draw_rectangle_filled(BrRenderer *renderer, int layer,
                                       BrVec2 position, BrVec2 size,
                                       int color);

/**
 * @brief Draws a rectangle outline.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param layer Draw order, lower layers are drawn first.
 * @param position Position of upper-left corner of the rectangle.
 * @param size Size of the rectangle in pixels.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Only supports axis-aligned rectangles.
 * @note Nothing is drawn until br_renderer_present() is called.
 */
void br_renderer_draw_rectangle_outlined(BrRenderer *renderer, int layer,
                                         BrVec2 position, BrVec2 size,
                                         int color);

/**
 * @brief Draws a texture at the specified position.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param layer Draw order, lower layers are drawn first.
 * @param position Position of upper-left corner of the texture.
 * @param texture Texture to draw. Must not be NULL, and must stay alive until
 * br_renderer_present() is called.
 *
 * @note Nothing is drawn until br_renderer_present() is called.
 */
void br_renderer_draw_texture(BrRenderer *renderer, int layer, BrVec2 position,
                              const BrTexture *texture);

/**
 * @brief Draws a region of a texture at the specified position.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param layer Draw order, lower layers are drawn first.
 * @param position Position of upper-left corner of the region.
 * @param region Texture region to draw. Its texture must not be NULL, and
 * must stay alive until br_renderer_present() is called.
 *
 * @note Nothing is drawn until br_renderer_present() is called.
 */
void br_renderer_draw_texture_region(BrRenderer *renderer, int layer,
                                     BrVec2 position, BrTextureRegion region);

/**
 * @brief Draws a single line of text at the specified position.
 *
 * Each glyph is drawn as a region of the font's atlas. Every character
 * advances by the glyph width plus the horizontal spacing, so
 * br_font_text_width() gives the drawn width. Characters the font has no
 * glyph for are left blank.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param layer Draw order, lower layers are drawn first.
 * @param font Font to use, set up with br_font_init(). Must not be NULL. Only
 * read during this call, but its atlas must stay alive until
 * br_renderer_present() is called.
 * @param text Text to render. Must not be NULL. Only read during this call.
 * @param position Position of the upper-left corner of the first character.
 *
 * @note Nothing is drawn until br_renderer_present() is called.
 * @note If the queue cannot fit the whole text this frame, none of it is
 * drawn.
 */
void br_renderer_draw_text(BrRenderer *renderer, int layer, const BrFont *font,
                           const char *text, BrVec2 position);

/**
 * @brief Draws everything recorded this frame and presents it to the window.
 *
 * Draws are made layer by layer, then the frame is shown and a new one
 * started. If the window is not ready for a new frame, the frame is dropped.
 *
 * @param renderer Renderer to present. Must not be NULL.
 */
void br_renderer_present(BrRenderer *renderer);

/**
 * @brief Resizes the renderer and recreates its buffers.
 *
 * @param renderer BrRenderer instance to resize. Passing NULL is safe and
 * does nothing.
 * @param width New width in pixels.
 * @param height New height in pixels.
 */
void br_renderer_resize(BrRenderer *renderer, int width, int height);

#endif // BORKA_RENDER_H
