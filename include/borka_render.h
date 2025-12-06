#ifndef BORKA_RENDER_H
#define BORKA_RENDER_H

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
 * @brief Draws a filled triangle.
 *
 * @param renderer Renderer to draw with. Passing NULL is safe and does
 * nothing.
 * @param v0 First vertex position in screen coordinates.
 * @param v1 Second vertex position in screen coordinates.
 * @param v2 Third vertex position in screen coordinates.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Vertices must be specified in counter-clockwise order, or the triangle
 * will not be rendered.
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_filled_triangle(BrRenderer *renderer, BrVec2 v0,
                                      BrVec2 v1, BrVec2 v2, int color);

/**
 * @brief Draws a filled rectangle.
 *
 * @param Renderer to draw with. Passing NULL is safe and does nothing.
 * @param x X coordinate of the top-left corner in screen space.
 * @param y Y coordinate of the top-left corner in screen space.
 * @param size Size of rectangle.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Only supports axis-aligned rectangles.
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_rectangle_filled(BrRenderer *renderer, int x, int y,
                                       int width, int height, int color);

/**
 * @brief Draws a rectangle outline.
 *
 * @param Renderer to draw with. Passing NULL is safe and does nothing.
 * @param x X coordinate of the top-left corner in screen space.
 * @param y Y coordinate of the top-left corner in screen space.
 * @param size Size of rectangle.
 * @param color Fill color in 32-bit ARGB format (0xAARRGGBB).
 *
 * @note Only supports axis-aligned rectangles.
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_rectangle_outlined(BrRenderer *renderer, int x, int y,
                                         int width, int height, int color);

/*
 * @brief Draws a texture at the specified position.
 *
 * @param renderer Renderer to draw with. Passing NULL is safe and does
 * nothing.
 * @param x X coordinate of the top-left corner in screen space.
 * @param y Y coordinate of the top-left corner in screen space.
 * @param texture The texture to draw. Passing NULL is safe and does nothing.
 *
 * @note Changes are not visible until br_renderer_present() is called.
 */
void br_renderer_draw_texture(BrRenderer *renderer, int x, int y,
                              const BrTexture *texture);

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
