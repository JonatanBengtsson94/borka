#include "components/components.h"
#include "systems.h"

static void bounce_ball(BrRegistry *registry, BrEntity ball,
                        BrEntity colliding_entity, Collider *ball_col,
                        Collider *colliding_entity_col) {
  Velocity *ball_vel = br_component_get(registry, COMPONENT_VELOCITY, ball);
  Position *ball_pos = br_component_get(registry, COMPONENT_POSITION, ball);
  Position *colliding_entity_pos =
      br_component_get(registry, COMPONENT_POSITION, colliding_entity);

  assert(ball_vel);
  assert(ball_pos);
  assert(colliding_entity_pos);

  if (ball_vel->vy > 0)
    ball_pos->y = colliding_entity_pos->y - ball_col->height;
  else
    ball_pos->y = colliding_entity_pos->y + colliding_entity_col->height;
  ball_vel->vy *= -1;
}

void system_collision_handling(BrRegistry *registry) {
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
      bounce_ball(registry, collision->entityB, collision->entityA, col_b,
                  col_a);
    }

    if (col_a->layer == LAYER_BALL && col_b->layer == LAYER_WALL) {
      BR_LOG_TRACE("Ball hit wall");
      bounce_ball(registry, collision->entityA, collision->entityB, col_a,
                  col_b);
    }

    br_component_remove(registry, query->current_entity, COMPONENT_COLLISION);
  }
}
