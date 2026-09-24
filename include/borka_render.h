#ifndef BORKA_RENDER_H
#define BORKA_RENDER_H

#include "borka_font.h"
#include "borka_math.h"
#include "borka_texture.h"

/**
 * @brief Opaque renderer handle.
 */
typedef struct BrRenderer BrRenderer;

/**
 * @brief Clears the entire framebuffer to the specified color.
 *
 * @param renderer The renderer to clear. Passing NULL is safe and does nothing.
 * @param color 32-bit ARGB color value (0xAARRGGBB format).
 *
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_clear(BrRenderer *renderer, int color);

/**
 * @brief Draws a filled rectangle.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param position Position of upper-left corner of the rectangle.
 * @param size Size of the rectangle in pixels.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Only supports axis-aligned rectangles.
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_rectangle_filled(BrRenderer *renderer, BrVec2 position,
                                       BrVec2 size, int color);

/**
 * @brief Draws a rectangle outline.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param position Position of upper-left corner of the rectangle.
 * @param size Size of the rectangle in pixels.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Only supports axis-aligned rectangles.
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_rectangle_outlined(BrRenderer *renderer, BrVec2 position,
                                         BrVec2 size, int color);

/**
 * @brief Draws a texture at the specified position.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param position Position of upper-left corner of the texture.
 * @param texture texture to draw. Must not be NULL.
 *
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_texture(BrRenderer *renderer, BrVec2 position,
                              const BrTexture *texture);

/**
 * @brief Draws a region of a texture at the specified position.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param position Position of upper-left corner of the region.
 * @param region Texture region to draw. Its texture must not be NULL.
 *
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_texture_region(BrRenderer *renderer, BrVec2 position,
                                     BrTextureRegion region);

/**
 * @brief Draws a single line of text at the specified position.
 *
 * Every character advances by the glyph width plus the horizontal spacing,
 * so br_font_text_width() gives the drawn width. Characters the font has no
 * glyph for are left blank.
 *
 * @param renderer Renderer to draw with. Must not be NULL.
 * @param font Font to use, set up with br_font_init(). Must not be NULL.
 * @param text Text to render. Must not be NULL.
 * @param position Position of the upper-left corner of the first character.
 *
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_text(BrRenderer *renderer, const BrFont *font,
                           const char *text, BrVec2 position);

/**
 * @brief Presents the rendered frame to the window.
 *
 * @param renderer Renderer to present. Passing NULL is safe and does
 * nothing.
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
