#include "components/components.h"
#include "constants.h"
#include "systems.h"
#include <math.h>

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
  br_entity_destroy(game->app->registry, brick);
  game->enemies_alive--;
  BR_LOG_TRACE("Enemies alive: %d", game->enemies_alive);
  if (game->enemies_alive == 0) {
    BR_LOG_INFO("Game is won");
    game->game_over = true;
  }
}

static void floor_hit(GameState *game) {
  BR_LOG_INFO("Game is lost");
  game->game_over = true;
}

static void bounce_ball(BrRegistry *registry, BrEntity ball, BrEntity hit,
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
    if (dx > 0)
      ball_p->x += overlap_x;
    else
      ball_p->x -= overlap_x;
    ball_v->vx *= -1;
  } else {
    if (dy > 0)
      ball_p->y += overlap_y;
    else
      ball_p->y -= overlap_y;
    ball_v->vy *= -1;
  }

  // Increase ball speed
  if (ball_v->vy < 0)
    ball_v->vy -= 5;
  else
    ball_v->vy += 5;
}

void system_collision_handling(GameState *game) {
  BrRegistry *registry = game->app->registry;
  BrQuery *query = br_query_begin(registry, SYSTEM_COLLISION_HANDLING);
  while (br_query_next(query)) {
    Collision *collision =
        (Collision *)br_query_get_component(query, COMPONENT_COLLISION);
    Collider *col_a = br_query_get_component(query, COMPONENT_COLLIDER);
    Collider *col_b =
        br_component_get(registry, COMPONENT_COLLIDER, collision->entityB);

    assert(collision);
    assert(col_a);
    assert(col_b);

    if (col_a->layer == LAYER_PADDLE && col_b->layer == LAYER_BALL) {
      BR_LOG_TRACE("Paddle hit ball");
      paddle_hit(registry, collision->entityB, collision->entityA, col_b,
                 col_a);
    }

    if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_WALL) {
      BR_LOG_TRACE("Ball hit wall");
      bounce_ball(registry, collision->entityA, collision->entityB, col_a,
                  col_b);
    }

    if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_BRICK) {
      BR_LOG_TRACE("Ball hit brick");
      bounce_ball(registry, collision->entityA, collision->entityB, col_a,
                  col_b);
      brick_hit(game, collision->entityB);
    }

    if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_FLOOR) {
      floor_hit(game);
    }

    br_component_remove(registry, query->current_entity, COMPONENT_COLLISION);
  }
}
