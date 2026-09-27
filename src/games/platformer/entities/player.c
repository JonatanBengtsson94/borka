#include "components/components.h"
#include "constants.h"
#include "entities.h"

BrEntity player_create(BrRegistry *registry, const Assets *assets) {
  assert(registry);
  assert(assets);
  BrEntity player = br_entity_create(registry);
  if (player == BR_INVALID_ENTITY) {
    BR_LOG_ERROR("Failed to create player entity");
    return BR_INVALID_ENTITY;
  }

  BrTexture *texture = assets->textures.player_texture;
  assert(texture);

  Position player_pos = {GAME_WIDTH / 2.0, GAME_HEIGHT / 2.0};
  Renderable player_ren = {
      .type = RENDERABLE_TEXTURE,
      .layer = RENDER_LAYER_WORLD,
      .texture.texture = texture,
  };
  Velocity player_vel = {0, 0};
  InputControlled player_control = {};

  if (!br_component_add(registry, player, COMPONENT_POSITION, &player_pos)) {
    BR_LOG_ERROR("Failed to add position component");
    goto error;
  }
  if (!br_component_add(registry, player, COMPONENT_RENDERABLE, &player_ren)) {
    BR_LOG_ERROR("Failed to add renderable component");
    goto error;
  }
  if (!br_component_add(registry, player, COMPONENT_VELOCITY, &player_vel)) {
    BR_LOG_ERROR("Failed to add velocity component");
    goto error;
  }
  if (!br_component_add(registry, player, COMPONENT_INPUT_CONTROLLED,
                        &player_control)) {
    BR_LOG_ERROR("Failed to add input controlled component");
    goto error;
  }

  BR_LOG_DEBUG("Created player entity at (%.1f, %.1f)", player_pos.x,
               player_pos.y);
  return player;

error:
  br_entity_destroy(registry, player);
  return BR_INVALID_ENTITY;
}
