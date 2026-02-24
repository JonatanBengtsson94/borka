#include "components/components.h"
#include "constants.h"
#include "entities.h"

void create_ball(BrRegistry *registry, BrTexture *texture) {
  BrEntity ball = br_entity_create(registry);
  Velocity ball_vel = {0, BALL_SPEED};
  Position ball_pos = {GAME_WIDTH / 2, GAME_HEIGHT / 2};
  Renderable ball_sprite = {.type = RENDERABLE_TEXTURE,
                            .texture = {.texture = texture}};
  Collider ball_col = {.size = {ball_sprite.texture.texture->size.x,
                                ball_sprite.texture.texture->size.y},
                       .layer = LAYER_BALL,
                       .mask = LAYER_WALL | LAYER_PADDLE | LAYER_BRICK |
                               LAYER_FLOOR};
  br_component_add(registry, ball, COMPONENT_POSITION, &ball_pos);
  br_component_add(registry, ball, COMPONENT_VELOCITY, &ball_vel);
  br_component_add(registry, ball, COMPONENT_RENDERABLE, &ball_sprite);
  br_component_add(registry, ball, COMPONENT_COLLIDER, &ball_col);
}
