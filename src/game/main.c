#include "borka.h"

int main() {
  BrApp *app = br_app_create("Breakout", 800, 600);

  BrVec2 a = {50, 10};
  BrVec2 b = {90, 80};
  BrVec2 c = {10, 80};

  while (br_window_poll_events(app->window)) {
    br_renderer_clear(app->renderer, 0xFFFFFFFF);
    br_renderer_draw_triangle(app->renderer, a, b, c, 0xFFFF0000);
    br_renderer_present(app->renderer);
  }

  br_app_destroy(app);
  return 0;
}
