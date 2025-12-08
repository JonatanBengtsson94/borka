#include "components/components.h"
#include "constants.h"
#include "entities.h"

void create_paddle(BrRegistry *registry, BrTexture *texture) {
  BrEntity paddle = br_entity_create(registry);
  Velocity paddle_vel = {0, 0};
  Position paddle_pos = {GAME_WIDTH / 2, GAME_HEIGHT - 10};
  Renderable paddle_sprite = {.type = RENDERABLE_SPRITE,
                              .sprite = {.texture = texture}};
  InputControlled paddle_input_control = {false, false};
  MovementConfig paddle_movement_conf = {PADDLE_SPEED};
  Collider paddle_col = {
      .size = {24, 1}, .layer = LAYER_PADDLE, .mask = LAYER_WALL | LAYER_BALL};
  br_component_add(registry, paddle, COMPONENT_POSITION, &paddle_pos);
  br_component_add(registry, paddle, COMPONENT_VELOCITY, &paddle_vel);
  br_component_add(registry, paddle, COMPONENT_RENDERABLE, &paddle_sprite);
  br_component_add(registry, paddle, COMPONENT_INPUT_CONTROLLED,
                   &paddle_input_control);
  br_component_add(registry, paddle, COMPONENT_MOVEMENT_CONFIG,
                   &paddle_movement_conf);
  br_component_add(registry, paddle, COMPONENT_COLLIDER, &paddle_col);
}
