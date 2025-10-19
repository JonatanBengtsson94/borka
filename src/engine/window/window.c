#include "window.h"
#include "logger.h"
#include <stddef.h>

#ifdef __linux__
#include "window/platform/wayland/wayland_window.h"

BrWindow *br_window_create(const BrWindowProps *props) {
  if (props->height <= 0 || props->width <= 0) {
    BR_LOG_ERROR("Invalid window extent");
    return NULL;
  }
  BrWindow *window = wayland_window_create(props);
  if (!window) {
    BR_LOG_ERROR("Failed to create wayland window");
    return NULL;
  }
  return window;
}

void br_window_destroy(BrWindow *window) { wayland_window_destroy(window); }

bool br_window_poll_events(BrWindow *window) {
  return wayland_window_poll_events(window);
}

#else
#error "Unsupported platform."
#endif
