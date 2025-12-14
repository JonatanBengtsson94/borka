#include "constants.h"
#include "game.h"

int main() {
  BrApp *app = br_app_create("Breakout", GAME_WIDTH, GAME_HEIGHT);
  if (!app)
    return -1;
  GameState game;
  game.app = app;

  if (!game_init(&game)) {
    return -1;
  }

  const double target_frame = 1.0 / MAX_FPS;
  double last = br_get_time();
  BrEvent e;

  while (!app->should_shutdown) {
    double now = br_get_time();
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

    // Frame cap
    double frame_time = br_get_time() - now;
    if (frame_time < target_frame) {
      int ns = (target_frame - frame_time) * 1e9;
      sleep(ns);
    }
  }

  game_shutdown(&game);
  return 0;
}
