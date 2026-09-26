#include "components/components.h"
#include "systems.h"

void system_movement(BrRegistry *registry, double delta_time) {
  assert(registry);

  BrQuery *query = br_query_begin(registry, SYSTEM_MOVEMENT);
  while (br_query_next(query)) {
    Position *p = (Position *)br_query_get_component(query, COMPONENT_POSITION);
    Velocity *v = (Velocity *)br_query_get_component(query, COMPONENT_VELOCITY);

    assert(p);
    assert(v);

    if (v->vx == 0 && v->vy == 0)
      continue;

    p->x += v->vx * delta_time;
    p->y += v->vy * delta_time;

    BR_LOG_TRACE("Moved entity to (%.2f, %.2f) with velocity (%.1f, %.1f)",
                 p->x, p->y, v->vx, v->vy);
  }
}
