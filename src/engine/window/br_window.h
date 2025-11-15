#ifndef BR_WINDOW_H
#define BR_WINDOW_H

#include "borka_window.h"

/**
 * @brief Creates a new BrWindow instance.
 *
 * @param title The title of the window.
 * @param width Width of the window in pixels.
 * @param height Height of the window in pixels.
 * @return Pointer to a newly created BrWindow instance, or NULL on failure.
 *
 * @note The window should be destroyed with br_window_destroy when no longer
 * needed.
 */
BrWindow *br_window_create(const char *title, int width, int height);

/**
 * @brief Destroys a window and releases associated resources.
 *
 * @param window The window to destroy. Safe to pass NULL.
 */
void br_window_destroy(BrWindow *window);

#endif // BR_WINDOW_H
