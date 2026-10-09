#include "components/components.h"
#include "constants.h"
#include "systems.h"

void system_gravity(BrRegistry *registry, double delta_time) {
  assert(registry);

  BrQuery *query = br_query_begin(registry, SYSTEM_GRAVITY);
  while (br_query_next(query)) {
    Body *b = br_query_get_component(query, COMPONENT_BODY);
    Velocity *v = br_query_get_component(query, COMPONENT_VELOCITY);

    assert(b);
    assert(v);

    if (!b->grounded) {
      v->vy += GRAVITY * delta_time;
      if (v->vy > MAX_FALL_SPEED)
        v->vy = MAX_FALL_SPEED;
    }
  }
}
