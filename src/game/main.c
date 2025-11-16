#include "borka.h"
#include <stdbool.h>

int main() {
  BrApp *app = br_app_create("Breakout", 200, 200);
  bool running = true;

  BrVec2 a = {50, 10};
  BrVec2 b = {90, 80};
  BrVec2 c = {10, 80};

  BrWindowEvent e;

  while (running) {
    // Poll for events
    while (br_window_poll_events(app->window, &e)) {
      switch (e.type) {
      case BR_WINDOW_EVENT_CLOSE:
        running = false;
        break;

      case BR_WINDOW_EVENT_RESIZE:
        br_renderer_resize(app->renderer, e.resize.width, e.resize.height);
        break;
      }
    }

    // Render
    br_renderer_clear(app->renderer, 0xFFFFFFFF);
    br_renderer_draw_triangle(app->renderer, a, b, c, 0xFFFF0000);
    br_renderer_present(app->renderer);
  }

  br_app_destroy(app);
  return 0;
}
