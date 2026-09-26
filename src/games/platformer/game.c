#include "game.h"
#include "components/components.h"
#include "entities/entities.h"
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

  if (create_player(game->app->registry, &game->assets) == BR_INVALID_ENTITY) {
    BR_LOG_ERROR("Failed to create player");
    goto error;
  }

  BR_LOG_DEBUG("Game initialized");
  return true;

error:
  game_shutdown(game);
  return false;
}

void game_update(Game *game) {
  system_render(game->app->registry, game->app->renderer);
}
