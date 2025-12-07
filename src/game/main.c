#define _POSIX_C_SOURCE 199309L

#include "game.h"
#include <time.h>

double get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main() {
  BrApp *app = br_app_create("Breakout", 200, 200);
  if (!app)
    return -1;
  GameState game;
  game.app = app;

  if (!game_init(&game)) {
    return -1;
  }

  double last = get_time();
  BrEvent e;

  while (!app->should_shutdown) {
    double now = get_time();
    double delta_time = now - last;
    last = now;

    while (br_window_poll_events(app->window, &e)) {
      switch (e.type) {
      case BR_EVENT_WINDOW_CLOSE:
        app->should_shutdown = true;
        break;

      case BR_EVENT_WINDOW_RESIZE:
        br_renderer_resize(app->renderer, e.data.resize.width,
                           e.data.resize.height);
        break;

      default:
        game_handle_event(&game, e);
        break;
      }
    }

    game_update(&game, delta_time);
  }

  game_shutdown(&game);
  return 0;
}
