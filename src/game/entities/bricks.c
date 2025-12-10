#include "borka.h"
#include "components/components.h"
#include "entities.h"

void create_bricks(GameState *game) {
  assert(game);
  assert(game->app->registry);
  assert(game->textures.brick);

  BrTexture *texture = game->textures.brick;
  BrRegistry *registry = game->app->registry;

  const int bricks_per_row = 10;
  const int rows = 2;
  const int padding = 16;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < bricks_per_row; j++) {
      Renderable brick_sprite = {.type = RENDERABLE_SPRITE,
                                 .sprite.texture = texture};
      Collider brick_collider = {
          .size.x = 16, .size.y = 16, .layer = LAYER_BRICK, .mask = LAYER_BALL};
      int x = padding - 8 + j * (brick_collider.size.x + padding);
      int y = i * (brick_collider.size.y + padding);
      BrEntity brick = br_entity_create(registry);
      Position brick_pos = {.x = x, .y = y};
      br_component_add(registry, brick, COMPONENT_POSITION, &brick_pos);
      br_component_add(registry, brick, COMPONENT_RENDERABLE, &brick_sprite);
      br_component_add(registry, brick, COMPONENT_COLLIDER, &brick_collider);
      game->enemies_alive++;
    }
  }
}
