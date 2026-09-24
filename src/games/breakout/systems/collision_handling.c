#include "components/components.h"
#include "constants.h"
#include "systems.h"
#include <math.h>

typedef enum {
  BOUNCE_NONE,       // Already moving away, so the velocity was left as is.
  BOUNCE_HORIZONTAL, // Off something beside the ball.
  BOUNCE_VERTICAL,   // Off something above or below the ball.
} Bounce;

// Restarts the ball's squash animation, flattened against whatever it hit.
static void squash_ball(GameState *game, BrEntity ball, Bounce bounce) {
  if (bounce == BOUNCE_NONE)
    return;

  Animator *a = br_component_get(game->app->registry, COMPONENT_ANIMATOR, ball);
  assert(a);

  if (bounce == BOUNCE_VERTICAL) {
    a->frames = game->animations.ball_squash_vertical;
    a->offsets = game->animations.ball_squash_vertical_offsets;
  } else {
    a->frames = game->animations.ball_squash_horizontal;
    a->offsets = game->animations.ball_squash_horizontal_offsets;
  }
  a->current_frame = 0;
  a->elapsed_time = 0.0f;
  a->finished = false;
  BR_LOG_TRACE("Squashing ball %s",
               bounce == BOUNCE_VERTICAL ? "vertically" : "horizontally");
}

static void paddle_hit(BrRegistry *registry, BrEntity ball, BrEntity paddle,
                       Collider *ball_col, Collider *paddle_col) {
  Position *ball_p = br_component_get(registry, COMPONENT_POSITION, ball);
  Position *paddle_p = br_component_get(registry, COMPONENT_POSITION, paddle);
  Velocity *ball_v = br_component_get(registry, COMPONENT_VELOCITY, ball);

  assert(ball_p);
  assert(paddle_p);
  assert(ball_v);

  float ball_cx = ball_p->x + (ball_col->size.x / 2.0);
  float paddle_cx = paddle_p->x + (paddle_col->size.x / 2.0);
  float dx = paddle_cx - ball_cx;
  float max_distance = (paddle_col->size.x + ball_col->size.x) / 2.0;
  float normalized_hit = dx / max_distance;
  BR_LOG_TRACE("Normalized hit: %f", normalized_hit);

  if (ball_v->vy > 0)
    ball_p->y = paddle_p->y - ball_col->size.y;
  else
    ball_p->y = paddle_p->y + paddle_col->size.y;

  float max_angle = 60.0;
  float angle = normalized_hit * max_angle * (PI / 180.0);
  float speed = sqrt(ball_v->vx * ball_v->vx + ball_v->vy * ball_v->vy);

  ball_v->vx = -speed * sin(angle);
  ball_v->vy = -speed * cos(angle);
  BR_LOG_TRACE("vx: %f, vy: %f", ball_v->vx, ball_v->vy);
}

static void brick_hit(GameState *game, BrEntity brick) {
  Position *pos =
      br_component_get(game->app->registry, COMPONENT_POSITION, brick);
  Brick *brick_data =
      br_component_get(game->app->registry, COMPONENT_BRICK, brick);
  assert(brick_data);
  assert(pos);

  BrTextureRegion *frames = NULL;
  switch (brick_data->type) {
  case BRICK_GREEN:
    frames = game->animations.brick_green_break;
    break;
  case BRICK_RED:
    frames = game->animations.brick_red_break;
    break;
  case BRICK_BLUE:
    frames = game->animations.brick_blue_break;
    break;
  }

  BrEntity anim = br_entity_create(game->app->registry);
  Animator animator = {
      .frames = frames,
      .number_of_frames = 3,
      .current_frame = 0,
      .frame_time = 0.05f,
      .elapsed_time = 0.0f,
      .on_end = ANIMATION_END_DESTROY,
      .finished = false,
  };
  Position anim_pos = {.x = pos->x, .y = pos->y};
  Renderable anim_ren = {.type = RENDERABLE_TEXTURE_REGION,
                         .layer = RENDER_LAYER_WORLD,
                         .region.region = animator.frames[0]};
  br_component_add(game->app->registry, anim, COMPONENT_POSITION, &anim_pos);
  br_component_add(game->app->registry, anim, COMPONENT_ANIMATOR, &animator);
  br_component_add(game->app->registry, anim, COMPONENT_RENDERABLE, &anim_ren);

  br_entity_destroy(game->app->registry, brick);
  game->score++;
  game->enemies_alive--;
  BR_LOG_TRACE("Enemies alive: %d", game->enemies_alive);
  if (game->enemies_alive == 0) {
    BR_LOG_INFO("Game is won");
    game->won = true;
    game->game_over = true;
  }
}

static void floor_hit(GameState *game) {
  BR_LOG_INFO("Game is lost");
  game->won = false;
  game->game_over = true;
}

static Bounce bounce_ball(BrRegistry *registry, BrEntity ball, BrEntity hit,
                          Collider *ball_col, Collider *hit_col) {
  Velocity *ball_v = br_component_get(registry, COMPONENT_VELOCITY, ball);
  Position *ball_p = br_component_get(registry, COMPONENT_POSITION, ball);
  Position *hit_p = br_component_get(registry, COMPONENT_POSITION, hit);

  assert(ball_v);
  assert(ball_p);
  assert(hit_p);

  float ball_cx = ball_p->x + (ball_col->size.x / 2.0);
  float ball_cy = ball_p->y + (ball_col->size.y / 2.0);
  float hit_cx = hit_p->x + (hit_col->size.x / 2.0);
  float hit_cy = hit_p->y + (hit_col->size.y / 2.0);

  float dx = ball_cx - hit_cx;
  float dy = ball_cy - hit_cy;

  float max_distance_x = (ball_col->size.x + hit_col->size.x) / 2.0;
  float max_distance_y = (ball_col->size.y + hit_col->size.y) / 2.0;

  float overlap_x = max_distance_x - fabsf(dx);
  float overlap_y = max_distance_y - fabsf(dy);

  if (overlap_x < overlap_y) {
    if (dx > 0) {
      ball_p->x += overlap_x;
      if (ball_v->vx < 0) {
        ball_v->vx *= -1;
        return BOUNCE_HORIZONTAL;
      }
    } else {
      ball_p->x -= overlap_x;
      if (ball_v->vx > 0) {
        ball_v->vx *= -1;
        return BOUNCE_HORIZONTAL;
      }
    }
  } else {
    if (dy > 0) {
      ball_p->y += overlap_y;
      if (ball_v->vy < 0) {
        ball_v->vy *= -1;
        ball_v->vy += 5;
        return BOUNCE_VERTICAL;
      }
    } else {
      ball_p->y -= overlap_y;
      if (ball_v->vy > 0) {
        ball_v->vy *= -1;
        ball_v->vy -= 5;
        return BOUNCE_VERTICAL;
      }
    }
  }
  return BOUNCE_NONE;
}

void system_collision_handling(GameState *game) {
  BrRegistry *registry = game->app->registry;
  BrQuery *query = br_query_begin(registry, SYSTEM_COLLISION_HANDLING);
  while (br_query_next(query)) {
    Collision *collision =
        (Collision *)br_query_get_component(query, COMPONENT_COLLISION);
    Collider *col_a = br_query_get_component(query, COMPONENT_COLLIDER);
    BrEntity entity_a = query->current_entity;

    assert(collision);
    assert(col_a);

    for (uint8_t i = 0; i < collision->count; i++) {
      BrEntity entity_b = collision->colliding_entities[i];
      Collider *col_b = br_component_get(registry, COMPONENT_COLLIDER, entity_b);

      assert(col_b);

      if (col_a->layer == LAYER_PADDLE && col_b->layer == LAYER_BALL) {
        BR_LOG_TRACE("Paddle hit ball");
        paddle_hit(registry, entity_b, entity_a, col_b, col_a);
        squash_ball(game, entity_b, BOUNCE_VERTICAL);
        br_play_sound_at_volume(game->sfx.paddle_hit, SFX_VOLUME);
      }

      if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_WALL) {
        BR_LOG_TRACE("Ball hit wall");
        squash_ball(game, entity_a,
                    bounce_ball(registry, entity_a, entity_b, col_a, col_b));
        br_play_sound_at_volume(game->sfx.wall_hit, SFX_VOLUME);
      }

      if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_BRICK) {
        BR_LOG_TRACE("Ball hit brick");
        squash_ball(game, entity_a,
                    bounce_ball(registry, entity_a, entity_b, col_a, col_b));
        brick_hit(game, entity_b);
        br_play_sound_at_volume(game->sfx.brick_hit, SFX_VOLUME);
      }

      if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_FLOOR) {
        floor_hit(game);
      }
    }

    br_component_remove(registry, entity_a, COMPONENT_COLLISION);
  }
}
