#include "components/components.h"
#include "systems.h"

void system_input(GameState *game, BrEvent e) {
  assert(game);
  assert(game->app);
  assert(game->app->registry);
  BrRegistry *registry = game->app->registry;

  bool pressed;
  if (e.type == BR_EVENT_KEY_PRESSED) {
    BR_LOG_TRACE("Key %d pressed", e.data.keycode);
    pressed = true;
  } else if (e.type == BR_EVENT_KEY_RELEASED) {
    BR_LOG_TRACE("Key %d released", e.data.keycode);
    pressed = false;
  }

  BrQuery *query = br_query_begin(registry, SYSTEM_INPUT);
  while (br_query_next(query)) {
    InputControlled *ic = (InputControlled *)br_query_get_component(
        query, COMPONENT_INPUT_CONTROLLED);

    assert(ic);

    switch (e.data.keycode) {
    case 30:
      ic->left_pressed = pressed;
      break;

    case 32:
      ic->right_pressed = pressed;
      break;

    case 57:
      if (pressed)
        game->is_paused = !game->is_paused;
      break;
    }
  }
}
