#include "systems.h"
#include "components/components.h"
#include <assert.h>

BrSystemId SYSTEM_INPUT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PLAYER_MOVEMENT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_RENDER = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PHYSICS = BR_INVALID_SYSTEM_ID;

bool systems_register(BrRegistry *registry) {
  BrComponentTypeId input_required[] = {COMPONENT_INPUT_CONTROLLED};
  BrComponentTypeId render_required[] = {COMPONENT_SPRITE, COMPONENT_POSITION};
  BrComponentTypeId physics_required[] = {COMPONENT_VELOCITY,
                                          COMPONENT_POSITION};
  BrComponentTypeId player_movement_required[] = {COMPONENT_INPUT_CONTROLLED,
                                                  COMPONENT_MOVEMENT_CONFIG,
                                                  COMPONENT_VELOCITY};

  SYSTEM_INPUT = br_register_system(registry, COMPONENT_INPUT_CONTROLLED,
                                    input_required, 1);
  SYSTEM_RENDER =
      br_register_system(registry, COMPONENT_SPRITE, render_required, 2);
  SYSTEM_PHYSICS =
      br_register_system(registry, COMPONENT_VELOCITY, physics_required, 2);
  SYSTEM_PLAYER_MOVEMENT = br_register_system(
      registry, COMPONENT_INPUT_CONTROLLED, player_movement_required, 3);

  BrSystemId ids[] = {SYSTEM_INPUT, SYSTEM_RENDER, SYSTEM_PHYSICS,
                      SYSTEM_PLAYER_MOVEMENT};
  size_t length = sizeof(ids) / sizeof(BrSystemId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_SYSTEM_ID) {
      BR_LOG_ERROR("Failed to register a system");
      return false;
    }
  }

  return true;
}

void system_input(BrRegistry *registry, BrEvent e) {
  assert(registry);

  bool pressed;
  if (e.type == BR_EVENT_KEY_PRESSED) {
    BR_LOG_TRACE("Key %d pressed", e.data.keycode);
    pressed = true;
  } else if (e.type == BR_EVENT_KEY_RELEASED) {
    BR_LOG_TRACE("Key %d released", e.data.keycode);
    pressed = false;
  }

  BrQuery *query = br_query_begin(registry, SYSTEM_INPUT);
  while (br_query_next(query)) {
    InputControlled *ic = (InputControlled *)br_query_get_component(
        query, COMPONENT_INPUT_CONTROLLED);

    assert(ic);

    switch (e.data.keycode) {
    case 30:
      ic->left_pressed = pressed;
      break;

    case 32:
      ic->right_pressed = pressed;
      break;
    }
  }
}

void system_render(BrRegistry *registry, BrRenderer *renderer) {
  assert(registry);
  assert(renderer);

  br_renderer_clear(renderer, 0xFF000000);

  BrQuery *query = br_query_begin(registry, SYSTEM_RENDER);
  while (br_query_next(query)) {
    Sprite *s = (Sprite *)br_query_get_component(query, COMPONENT_SPRITE);
    Position *p = (Position *)br_query_get_component(query, COMPONENT_POSITION);

    assert(s);
    assert(s->texture);
    assert(p);

    br_renderer_draw_texture(renderer, p->x, p->y, s->texture);
  }
  br_renderer_present(renderer);
}

void system_physics(BrRegistry *registry, double delta_time) {
  assert(registry);

  BrQuery *query = br_query_begin(registry, SYSTEM_PHYSICS);
  while (br_query_next(query)) {
    Position *p = (Position *)br_query_get_component(query, COMPONENT_POSITION);
    Velocity *v = (Velocity *)br_query_get_component(query, COMPONENT_VELOCITY);

    assert(p);
    assert(v);

    p->x += v->vx * delta_time;
    p->y += v->vy * delta_time;
  }
}

void system_player_movement(BrRegistry *registry) {
  BrQuery *query = br_query_begin(registry, SYSTEM_PLAYER_MOVEMENT);
  while (br_query_next(query)) {
    InputControlled *ic =
        br_query_get_component(query, COMPONENT_INPUT_CONTROLLED);
    MovementConfig *mc =
        br_query_get_component(query, COMPONENT_MOVEMENT_CONFIG);
    Velocity *v = br_query_get_component(query, COMPONENT_VELOCITY);

    assert(ic);
    assert(mc);
    assert(v);

    int horizontal = 0;
    float speed = mc->move_speed;

    if (ic->left_pressed && !ic->right_pressed)
      horizontal = -1;
    else if (ic->right_pressed && !ic->left_pressed)
      horizontal = 1;
    else
      horizontal = 0;

    v->vx = horizontal * speed;
  }
}
