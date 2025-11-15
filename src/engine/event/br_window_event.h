#ifndef BR_WINDOW_EVENT_H
#define BR_WINDOW_EVENT_H

#include "borka_events.h"
#include <stdbool.h>

/**
 * @brief Polls the next available window event.
 *
 * If an event is available, it is written to the out_event.
 *
 * @param out_event Pointer to write the next event into.
 * @return true if an event was returned, false if there was no available event.
 */
bool br_window_event_poll(BrWindowEvent *out_event);

/**
 * @brief Pushes a window event into the event system.
 *
 * @param event The event to push.
 * @return true if the event was added, false if if failed.
 */
bool br_window_event_push(const BrWindowEvent *event);

#endif // BR_WINDOW_EVENT_H
