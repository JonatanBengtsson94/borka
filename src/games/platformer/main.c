#include "borka_app.h"
#include "constants.h"
#include "game.h"
#include <borka.h>

int main() {
  BrApp *app = br_app_create("Platformer", GAME_WIDTH, GAME_HEIGHT);
  if (!app)
    return -1;

  Game game = {.app = app};
  if (!game_init(&game)) {
    br_app_destroy(app);
    return -1;
  }

  while (!app->should_shutdown) {
    BrEvent e;
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
        // TODO: Handle game events
        break;
      }
    }

    game_update(&game);
  }

  game_shutdown(&game);
  br_app_destroy(app);
  return 0;
}
