#ifndef BORKA_RENDER_H
#define BORKA_RENDER_H

#include "borka_math.h"

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
 * @brief Draws a filled triangle.
 *
 * @param target the renderer to draw with. Passing NULL is safe and does
 * nothing.
 * @param v0 First vertex position in screen coordinates.
 * @param v1 Second vertex position in screen coordinates.
 * @param v2 Third vertex position in screen coordinates.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Vertices must be specified in counter-clockwise order, or the triangle
 * will not be rendered.
 * @note Changes are not visable until br_renderer_present() is called.
 */
void br_renderer_draw_triangle(BrRenderer *renderer, BrVec2 v0, BrVec2 v1,
                               BrVec2 v2, int color);
void br_renderer_draw_quad();

/**
 * @brief Presents the rendered frame to the window.
 *
 * @param renderer The renderer to present. Passing NULL is safe and does
 * nothing.
 */
void br_renderer_present(BrRenderer *renderer);

#endif // BORKA_RENDER_H
