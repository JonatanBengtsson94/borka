#include "components/components.h"
#include "constants.h"
#include "systems.h"

void system_player_movement(BrRegistry *registry) {
  BrQuery *query = br_query_begin(registry, SYSTEM_PLAYER_MOVEMENT);
  while (br_query_next(query)) {
    InputControlled *ic =
        br_query_get_component(query, COMPONENT_INPUT_CONTROLLED);
    MovementConfig *mc =
        br_query_get_component(query, COMPONENT_MOVEMENT_CONFIG);
    Velocity *v = br_query_get_component(query, COMPONENT_VELOCITY);
    Position *p = br_query_get_component(query, COMPONENT_POSITION);
    Collider *c = br_query_get_component(query, COMPONENT_COLLIDER);

    assert(ic);
    assert(mc);
    assert(v);
    assert(p);
    assert(c);

    int horizontal = 0;
    float speed = mc->move_speed;

    if (ic->left_pressed && !ic->right_pressed)
      horizontal = -1;
    else if (ic->right_pressed && !ic->left_pressed)
      horizontal = 1;
    else
      horizontal = 0;

    v->vx = horizontal * speed;

    if (p->x < 0)
      p->x = 0;
    if (p->x > GAME_WIDTH - c->size.x)
      p->x = GAME_WIDTH - c->size.x;
  }
}
