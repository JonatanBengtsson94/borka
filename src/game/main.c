#include "borka.h"
#include "render/render.h"
#include "window/window.h"

int main() {
  int running = 1;
  if (br_init("breakout") != 0) {
    return -1;
  }

  BrWindowProps window_props = {800, 600, "Breakout"};
  BrWindow *window = br_window_create(&window_props);
  BrRenderer *renderer = br_render_create(window);

  while (running) {
    br_window_poll_events(window);
    br_render_draw(renderer);
  }

  if (window) {
    br_window_destroy(window);
  }

  br_shutdown();

  return 0;
}
