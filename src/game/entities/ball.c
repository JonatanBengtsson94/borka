#include "components/components.h"
#include "entities.h"

void create_ball(BrRegistry *registry, BrTexture *texture) {
  BrEntity ball = br_entity_create(registry);
  Position ball_pos = {100, 100};
  Velocity ball_vel = {0, 50};
  Renderable ball_sprite = {.type = RENDERABLE_SPRITE,
                            .sprite = {.texture = texture}};
  Collider ball_col = {.width = 8,
                       .height = 8,
                       .layer = LAYER_BALL,
                       .mask = LAYER_WALL | LAYER_PADDLE | LAYER_BRICK |
                               LAYER_FLOOR};
  br_component_add(registry, ball, COMPONENT_POSITION, &ball_pos);
  br_component_add(registry, ball, COMPONENT_VELOCITY, &ball_vel);
  br_component_add(registry, ball, COMPONENT_RENDERABLE, &ball_sprite);
  br_component_add(registry, ball, COMPONENT_COLLIDER, &ball_col);

  // Left Wall
  BrEntity left_wall = br_entity_create(registry);
  Position left_wall_pos = {0, 0};
  Collider left_wall_col = {
      .width = 1, .height = 200, .layer = LAYER_WALL, .mask = LAYER_BALL};
  br_component_add(registry, left_wall, COMPONENT_POSITION, &left_wall_pos);
  br_component_add(registry, left_wall, COMPONENT_COLLIDER, &left_wall_col);
}
