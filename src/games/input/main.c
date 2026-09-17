#include "logger/br_logger.h"
#include "window/br_window.h"

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 180

int main() {
  if (!br_logger_init("Input"))
    return 1;

  BrWindow *window = br_window_create("Input", WINDOW_WIDTH, WINDOW_HEIGHT);
  if (!window) {
    br_logger_shutdown();
    return 1;
  }

  bool should_shutdown = false;
  BrEvent e;
  while (!should_shutdown) {
    while (br_window_poll_events(window, &e)) {
      switch (e.type) {
      case BR_EVENT_WINDOW_CLOSE:
        should_shutdown = true;
        break;

      case BR_EVENT_KEY_PRESSED:
        BR_LOG_INFO("Key pressed: %d", e.data.keycode);
        break;

      case BR_EVENT_KEY_RELEASED:
        BR_LOG_INFO("Key released: %d", e.data.keycode);
        break;

      default:
        break;
      }
    }
  }

  br_window_destroy(window);
  br_logger_shutdown();
  return 0;
}
