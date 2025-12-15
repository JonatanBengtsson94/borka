#include "borka.h"
#include "components/components.h"
#include "entities.h"

void create_bricks(GameState *game) {
  assert(game);
  assert(game->app->registry);
  assert(game->textures.brick_green && game->textures.brick_blue &&
         game->textures.brick_red);

  BrRegistry *registry = game->app->registry;

  const int bricks_per_row = 17;
  const int rows = 4;
  const int padding = 2;

  BrTexture *texture = game->textures.brick_red;
  for (int i = 0; i < rows; i++) {
    if (i == 1) {
      texture = game->textures.brick_blue;
    } else if (i > 1) {
      texture = game->textures.brick_green;
    }
    for (int j = 0; j < bricks_per_row; j++) {
      Renderable brick_sprite = {.type = RENDERABLE_SPRITE,
                                 .sprite.texture = texture};
      Collider brick_collider = {.size.x = brick_sprite.sprite.texture->size.x,
                                 .size.y = brick_sprite.sprite.texture->size.y,
                                 .layer = LAYER_BRICK,
                                 .mask = LAYER_BALL};
      int x = 8 + j * (brick_collider.size.x + padding);
      int y = 32 + i * (brick_collider.size.y + padding);
      BrEntity brick = br_entity_create(registry);
      Position brick_pos = {.x = x, .y = y};
      br_component_add(registry, brick, COMPONENT_POSITION, &brick_pos);
      br_component_add(registry, brick, COMPONENT_RENDERABLE, &brick_sprite);
      br_component_add(registry, brick, COMPONENT_COLLIDER, &brick_collider);
      game->enemies_alive++;
    }
  }
}
