#include "borka.h"
#include "components/components.h"
#include "constants.h"
#include "entities.h"

bool ground_create(BrRegistry *registry, const Assets *assets) {
  assert(registry);
  assert(assets);
  BrTexture *texture = assets->textures.ground_texture;
  assert(texture);

  int tile_width = texture->size.x;
  // Round up so the last tile covers the right edge of the screen.
  int tile_count = (GAME_WIDTH + tile_width - 1) / tile_width;
  assert(tile_count <= MAX_ENTITIES);

  Renderable ground_ren = {.type = RENDERABLE_TEXTURE,
                           .layer = RENDER_LAYER_TERRAIN,
                           .texture.texture = texture};

  // Every tile created so far, so a failure can undo the whole row.
  BrEntity tiles[MAX_ENTITIES];
  int created = 0;

  for (int i = 0; i < tile_count; i++) {
    BrEntity tile = br_entity_create(registry);
    if (tile == BR_INVALID_ENTITY) {
      BR_LOG_ERROR("Failed to create ground tile entity");
      goto error;
    }
    tiles[created++] = tile;

    Position tile_pos = {i * tile_width, GROUND_Y};

    if (!br_component_add(registry, tile, COMPONENT_POSITION, &tile_pos)) {
      BR_LOG_ERROR("Failed to add position component");
      goto error;
    }
    if (!br_component_add(registry, tile, COMPONENT_RENDERABLE,
                          &ground_ren)) {
      BR_LOG_ERROR("Failed to add renderable component");
      goto error;
    }
    BR_LOG_TRACE("Created ground tile %u at (%.1f, %.1f)", tile, tile_pos.x,
                 tile_pos.y);
  }

  BR_LOG_DEBUG("Created %d ground tiles at y %d", tile_count, GROUND_Y);
  return true;

error:
  BR_LOG_DEBUG("Destroying %d ground tiles after failure", created);
  for (int i = 0; i < created; i++)
    br_entity_destroy(registry, tiles[i]);
  return false;
}
