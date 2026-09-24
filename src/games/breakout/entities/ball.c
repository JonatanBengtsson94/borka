#include "components/components.h"
#include "constants.h"
#include "entities.h"

void create_ball(GameState *game) {
  assert(game);
  BrRegistry *registry = game->app->registry;
  BrTexture *texture = game->textures.ball;
  const BrTextureRegion *trail = game->textures.trail;

  BrEntity ball = br_entity_create(registry);
  Velocity ball_vel = {0, BALL_SPEED};
  Position ball_pos = {GAME_WIDTH / 2, GAME_HEIGHT / 2};

  // Every point starts at the ball's center, so the segments sit hidden
  // behind the ball until it has moved.
  Trail ball_trail = {.center_offset = {texture->size.x / 2.0f,
                                        texture->size.y / 2.0f}};
  Position center = {ball_pos.x + ball_trail.center_offset.x,
                     ball_pos.y + ball_trail.center_offset.y};
  for (int i = 0; i < TRAIL_POINTS; i++)
    ball_trail.points[i] = center;

  for (int i = 0; i < TRAIL_LENGTH; i++) {
    BrEntity segment = br_entity_create(registry);
    Renderable segment_sprite = {.type = RENDERABLE_TEXTURE_REGION,
                                 .layer = RENDER_LAYER_BEHIND,
                                 .region.region = trail[i]};
    br_component_add(registry, segment, COMPONENT_POSITION, &center);
    br_component_add(registry, segment, COMPONENT_RENDERABLE, &segment_sprite);
    ball_trail.segments[i] = segment;
  }

  // A region rather than the whole texture, so the squash animation can swap
  // in frames from another texture.
  Renderable ball_sprite = {
      .type = RENDERABLE_TEXTURE_REGION,
      .layer = RENDER_LAYER_WORLD,
      .region.region = {.texture = texture, .size = texture->size}};
  // Idle until a bounce restarts it, resting on the round ball frame.
  Animator ball_squash = {.frames = game->animations.ball_squash_vertical,
                          .offsets = game->animations.ball_squash_vertical_offsets,
                          .number_of_frames = 3,
                          .current_frame = 2,
                          .frame_time = BALL_SQUASH_FRAME_TIME,
                          .on_end = ANIMATION_END_HOLD,
                          .finished = true};
  Collider ball_col = {.size = {texture->size.x, texture->size.y},
                       .layer = LAYER_BALL,
                       .mask = LAYER_WALL | LAYER_PADDLE | LAYER_BRICK |
                               LAYER_FLOOR};
  br_component_add(registry, ball, COMPONENT_POSITION, &ball_pos);
  br_component_add(registry, ball, COMPONENT_VELOCITY, &ball_vel);
  br_component_add(registry, ball, COMPONENT_RENDERABLE, &ball_sprite);
  br_component_add(registry, ball, COMPONENT_COLLIDER, &ball_col);
  br_component_add(registry, ball, COMPONENT_TRAIL, &ball_trail);
  br_component_add(registry, ball, COMPONENT_ANIMATOR, &ball_squash);
}
