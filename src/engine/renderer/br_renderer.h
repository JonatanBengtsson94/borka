#ifndef BR_RENDERER_H
#define BR_RENDERER_H

#include "math/br_math.h"
#include "window/br_window.h"

/**
 * @brief Opaque renderer handle.
 */
typedef struct BrRenderer BrRenderer;

/**
 * @brief Creates a renderer for the specified window.
 *
 * @param window The window to render to. Passing NULL is safe and does nothing.
 * @return Pointer to a newly created BrRenderer instance, or NULL on failure.
 *
 * @note The renderer should be destroyed with br_renderer_destroy() when no
 * longer needed.
 */
BrRenderer *br_renderer_create(BrWindow *window);

/**
 * @brief Destroys a renderer and frees associated resources.
 *
 * @param renderer The renderer to destroy. Safe to pass NULL.
 */
void br_renderer_destroy(BrRenderer *renderer);

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

#endif // BR_RENDERER_H
