#include "scenes.h"
#include "entities/entities.h"

bool scene_load(Game *game, SceneId scene) {
  assert(game);
  assert(game->app);
  assert(scene != SCENE_NONE);
  BR_LOG_DEBUG("Loading scene %d", scene);

  switch (scene) {
  case SCENE_NONE:
    // Ruled out by the assert above.
    break;

  case SCENE_SANDBOX:
    if (player_create(game->app->registry, &game->assets) ==
        BR_INVALID_ENTITY) {
      BR_LOG_ERROR("Failed to create player");
      goto error;
    }

    if (!ground_create(game->app->registry, &game->assets)) {
      BR_LOG_ERROR("Failed to create ground");
      goto error;
    }
    break;
  }

  BR_LOG_DEBUG("Loaded scene %d", scene);
  return true;

error:
  BR_LOG_ERROR("Failed to load scene %d", scene);
  return false;
}
