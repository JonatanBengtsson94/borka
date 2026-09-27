#include "assets.h"

void assets_destroy(Assets *assets) {
  assert(assets);
  BR_LOG_DEBUG("Destroying assets");
  if (assets->textures.player_texture) {
    br_texture_destroy(assets->textures.player_texture);
    assets->textures.player_texture = NULL;
  }
  if (assets->textures.ground_texture) {
    br_texture_destroy(assets->textures.ground_texture);
    assets->textures.ground_texture = NULL;
  }
}

bool assets_load(Assets *assets) {
  assert(assets);
  BR_LOG_DEBUG("Loading assets");

  assets->textures.player_texture =
      br_texture_create("assets/textures/placeholder_player.png");
  if (!assets->textures.player_texture) {
    BR_LOG_ERROR("Failed to load player texture");
    goto error;
  }
  BR_LOG_TRACE("Loaded player texture");

  assets->textures.ground_texture =
      br_texture_create("assets/textures/ground_placeholder.png");
  if (!assets->textures.ground_texture) {
    BR_LOG_ERROR("Failed to load ground texture");
    goto error;
  }
  BR_LOG_TRACE("Loaded ground texture");

  return true;

error:
  assets_destroy(assets);
  return false;
}
