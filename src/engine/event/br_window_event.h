#ifndef BR_WINDOW_EVENT_H
#define BR_WINDOW_EVENT_H

#include "borka_events.h"
#include <stdbool.h>

bool br_window_event_poll(BrWindowEvent *out_event);
bool br_window_event_push(const BrWindowEvent *event);

#endif // BR_WINDOW_EVENT_H
