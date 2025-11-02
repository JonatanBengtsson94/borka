#include "borka.h"
#include "renderer/br_renderer.h"
#include "window/br_window.h"
#include <time.h>

int main() {
  if (br_init("breakout") != 0) {
    return -1;
  }

  BrWindowProps window_props = {600, 800, "Breakout"};
  BrWindow *window = br_window_create(&window_props);
  BrRenderer *renderer = br_renderer_create(window);

  BrVec2 a = {50, 10};
  BrVec2 b = {90, 80};
  BrVec2 c = {10, 80};

  int frame_count = 0;
  struct timespec sleep_time = {0, 16666666};
  while (frame_count < 300) {
    br_window_poll_events(window);

    br_renderer_clear(renderer, 0xFFFFFFFF);
    br_renderer_draw_triangle(renderer, a, b, c, 0xFFFF0000);
    br_renderer_present(renderer);

    nanosleep(&sleep_time, NULL);
    frame_count++;
  }

  br_renderer_destroy(renderer);
  br_window_destroy(window);
  br_shutdown();

  return 0;
}
