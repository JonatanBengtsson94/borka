#ifndef BR_RENDERER_H
#define BR_RENDERER_H

#include "borka_render.h"
#include "borka_window.h"

/**
 * @brief Creates a new instance of BrRenderer.
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

#endif // BR_RENDERER_H
