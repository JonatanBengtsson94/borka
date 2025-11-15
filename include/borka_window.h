#ifndef BORKA_WINDOW_H
#define BORKA_WINDOW_H

#include "borka_events.h"
#include <stdbool.h>

/**
 * @brief Opaque window handle.
 */
typedef struct BrWindow BrWindow;

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

#endif
