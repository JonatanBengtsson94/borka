#include "components/components.h"
#include "constants.h"
#include "entities.h"

void create_paddle(GameState *game) {
  assert(game);
  BrRegistry *registry = game->app->registry;
  BrTexture *texture = game->textures.paddle;

  BrEntity paddle = br_entity_create(registry);
  Velocity paddle_vel = {0, 0};
  Position paddle_pos = {GAME_WIDTH / 2, GAME_HEIGHT - 10};
  // A region, as the recoil animation sets the sprite through its frames.
  Renderable paddle_sprite = {
      .type = RENDERABLE_TEXTURE_REGION,
      .layer = RENDER_LAYER_WORLD,
      .region.region = game->animations.paddle_recoil[2]};
  // Idle until the ball hits the paddle, resting on the undipped frame.
  Animator paddle_recoil = {.frames = game->animations.paddle_recoil,
                            .offsets = game->animations.paddle_recoil_offsets,
                            .number_of_frames = 3,
                            .current_frame = 2,
                            .frame_time = PADDLE_RECOIL_FRAME_TIME,
                            .on_end = ANIMATION_END_HOLD,
                            .finished = true};
  InputControlled paddle_input_control = {false, false};
  MovementConfig paddle_movement_conf = {PADDLE_SPEED};
  Collider paddle_col = {.size = {texture->size.x, 1},
                         .layer = LAYER_PADDLE,
                         .mask = LAYER_WALL | LAYER_BALL};
  br_component_add(registry, paddle, COMPONENT_POSITION, &paddle_pos);
  br_component_add(registry, paddle, COMPONENT_VELOCITY, &paddle_vel);
  br_component_add(registry, paddle, COMPONENT_RENDERABLE, &paddle_sprite);
  br_component_add(registry, paddle, COMPONENT_INPUT_CONTROLLED,
                   &paddle_input_control);
  br_component_add(registry, paddle, COMPONENT_MOVEMENT_CONFIG,
                   &paddle_movement_conf);
  br_component_add(registry, paddle, COMPONENT_COLLIDER, &paddle_col);
  br_component_add(registry, paddle, COMPONENT_ANIMATOR, &paddle_recoil);
}
