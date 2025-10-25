#ifndef RENDERER_H
#define RENDERER_H

#include "window/window.h"

/**
 * @brief Opaque handle to platform-independent renderer.
 */
typedef struct BrRenderer BrRenderer;

/**
 * @brief Creates a renderer for the provided window.
 *
 * @param window Pointer to the window the renderer should render to.
 * @return Pointer to a newly created BrRenderer instance, or NULL on failure.
 */
BrRenderer *br_renderer_create(BrWindow *window);

/**
 * @brief Presents the current rendered frame.
 *
 * @param renderer Pointer to the renderer that should render.
 */
void br_renderer_present(BrRenderer *renderer);

#endif // RENDERER_H
