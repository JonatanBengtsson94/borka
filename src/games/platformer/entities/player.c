#include "components/components.h"
#include "constants.h"
#include "entities.h"

BrEntity create_player(BrRegistry *registry, const Assets *assets) {
  assert(registry);
  assert(assets);
  BrEntity player = br_entity_create(registry);
  assert(player != BR_INVALID_ENTITY);

  BrTexture *texture = assets->textures.player_texture;
  assert(texture);

  Position player_pos = {GAME_WIDTH / 2, GAME_HEIGHT / 2};
  Renderable player_ren = {
      .type = RENDERABLE_TEXTURE,
      .layer = RENDER_LAYER_WORLD,
      .texture.texture = texture,
  };

  if (!br_component_add(registry, player, COMPONENT_POSITION, &player_pos)) {
    BR_LOG_ERROR("Failed to add position component");
    return BR_INVALID_ENTITY;
  }
  if (!br_component_add(registry, player, COMPONENT_RENDERABLE, &player_ren)) {
    BR_LOG_ERROR("Failed to add renderable component");
    return BR_INVALID_ENTITY;
  }

  BR_LOG_DEBUG("Created player entity at (%.1f, %.1f)", player_pos.x,
               player_pos.y);
  return player;
}
