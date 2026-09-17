#include "logger/br_logger.h"
#include "window/br_window.h"

int main() {
  if (!br_logger_init("Window"))
    return 1;

  BrWindow *window = br_window_create("Window", 320, 180);
  if (!window) {
    br_logger_shutdown();
    return 1;
  }

  bool should_shutdown = false;
  BrEvent e;
  while (!should_shutdown) {
    while (br_window_poll_events(window, &e)) {
      if (e.type == BR_EVENT_WINDOW_CLOSE)
        should_shutdown = true;
    }
  }

  br_window_destroy(window);
  br_logger_shutdown();
  return 0;
}
