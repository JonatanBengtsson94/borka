#ifndef BR_WINDOW_H
#define BR_WINDOW_H

#include "event/br_window_event.h"
#include <stdbool.h>

/**
 * @brief Opaque window handle.
 */
typedef struct BrWindow BrWindow;

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

/**
 * @brief Polls for window events (non-blocking).
 *
 * This function processess all pending platform events (such as buffer
 * releases, input events, etc.) and returns the next queued window event if
 * available. It should be called once per frame in the main loop.
 *
 * @param window The window to poll events from.
 * @param out_event The BrWindowEvent structure to fill with event data.
 * @return true if an event was retrieved, false if the queue was empty.
 */
bool br_window_poll_events(BrWindow *window, BrWindowEvent *out_event);

#endif // BR_WINDOW_H
