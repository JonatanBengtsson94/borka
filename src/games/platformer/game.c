#include "game.h"
#include "components/components.h"
#include "scenes/scenes.h"
#include "systems/systems.h"

void game_shutdown(Game *game) {
  assert(game);
  BR_LOG_DEBUG("Shutting down game");
  assets_destroy(&game->assets);
}

bool game_init(Game *game) {
  assert(game);
  assert(game->app);
  BR_LOG_DEBUG("Initializing game");

  if (!components_register(game->app->registry)) {
    BR_LOG_ERROR("Failed to register components");
    goto error;
  }

  if (!systems_register(game->app->registry)) {
    BR_LOG_ERROR("Failed to register systems");
    goto error;
  }

  if (!assets_load(&game->assets)) {
    BR_LOG_ERROR("Failed to load assets");
    goto error;
  }

  // TODO: Should load main menu when it exists
  if (!scene_load(game, SCENE_SANDBOX)) {
    BR_LOG_ERROR("Failed to load sandbox scene");
    goto error;
  }

  BR_LOG_DEBUG("Game initialized");
  return true;

error:
  game_shutdown(game);
  return false;
}

void game_handle_event(Game *game, BrEvent event) {
  assert(game);
  assert(game->app);

  if (event.type != BR_EVENT_KEY_PRESSED &&
      event.type != BR_EVENT_KEY_RELEASED) {
    BR_LOG_TRACE("Ignoring non-key event of type %d", event.type);
    return;
  }

  system_input(game->app->registry, event);
}

void game_update(Game *game, double delta_time) {
  system_player_actions(game->app->registry);
  system_movement(game->app->registry, delta_time);
  system_render(game->app->registry, game->app->renderer);
}
