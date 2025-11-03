#ifndef BR_WINDOW_H
#define BR_WINDOW_H

#include <stdbool.h>

/**
 * @brief Opaque window handle.
 */
typedef struct BrWindow BrWindow;

/**
 * @brief Creates a new window with the specified properties.
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

/**
 * @brief Polls and processes window events without blocking.
 *
 * This function checks for events from the window system (such as resize,
 * input, or close request) and dispatches them to appropriate handlers.
 *
 * @param window Window to poll events for. Passin NULL is safe and does
 * nothing.
 * @return true if the window should continue running, false if the window
 * received a close request, an error occurred, or the connection to the display
 * was lost.
 */
bool br_window_poll_events(BrWindow *window);

#endif // BR_WINDOW_H
