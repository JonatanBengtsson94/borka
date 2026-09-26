#include "components/components.h"
#include "constants.h"
#include "systems.h"

void system_player_actions(BrRegistry *registry) {
  assert(registry);

  BrQuery *query = br_query_begin(registry, SYSTEM_PLAYER_ACTIONS);
  while (br_query_next(query)) {
    InputControlled *ic =
        br_query_get_component(query, COMPONENT_INPUT_CONTROLLED);
    Velocity *v = br_query_get_component(query, COMPONENT_VELOCITY);

    assert(ic);
    assert(v);

    int horizontal = 0;

    if (ic->left_pressed && !ic->right_pressed)
      horizontal = -1;
    else if (ic->right_pressed && !ic->left_pressed)
      horizontal = 1;
    else
      horizontal = 0;

    v->vx = horizontal * BASE_SPEED;
    // TODO: Jumping
  }
}
