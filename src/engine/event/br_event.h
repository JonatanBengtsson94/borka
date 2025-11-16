#ifndef BR_WINDOW_EVENT_H
#define BR_WINDOW_EVENT_H

#include "borka_events.h"
#include <stdbool.h>

bool br_event_poll(BrEvent *out_event);
bool br_event_push(const BrEvent *event);

#endif // BR_WINDOW_EVENT_H
