#include "borka_ecs.h"
#include "borka_events.h"
#include "borka_log.h"
#include <stdbool.h>

void system_input(BrRegistry *registry, BrEvent e) {
  BrMovementInput *input = &registry->movement_inputs[0];
  bool pressed;

  if (e.type == BR_EVENT_KEY_PRESSED) {
    BR_LOG_TRACE("Key %d pressed", e.data.keycode);
    pressed = true;
  } else if (e.type == BR_EVENT_KEY_RELEASED) {
    BR_LOG_TRACE("Key %d released", e.data.keycode);
    pressed = false;
  }

  switch (e.data.keycode) {
  case 30:
    input->left_pressed = pressed;
    break;

  case 32:
    input->right_pressed = pressed;
    break;
  }
}

void system_render(BrRegistry *registry, BrRenderer *renderer) {
  br_renderer_clear(renderer, 0xFF000000);
  for (int i = 0; i < registry->count; i++) {
    if ((registry->masks[i] & (COMPONENT_POSITION | COMPONENT_SPRITE)) ==
        (COMPONENT_POSITION | COMPONENT_SPRITE)) {
      br_renderer_draw_texture(renderer, registry->positions[i].x,
                               registry->positions[i].y,
                               registry->sprites[i].texture);
      BR_LOG_TRACE("Entity: %u got renderered at position %fx%f", i,
                   registry->positions[i].x, registry->positions[i].y);
    }
  }
  br_renderer_present(renderer);
}

void system_movement(BrRegistry *registry, double delta_time) {
  for (int i = 0; i < registry->count; i++) {
    if ((registry->masks[i] & (COMPONENT_POSITION | COMPONENT_VELOCITY)) ==
        (COMPONENT_POSITION | COMPONENT_VELOCITY)) {
      registry->positions[i].x += registry->velocities[i].dx * delta_time;
      registry->positions[i].y += registry->velocities[i].dy * delta_time;
      BR_LOG_TRACE("Entity: %u got new position %fx%f", i,
                   registry->positions[i].x, registry->positions[i].y);
    }
  }
}

void system_player_controller(BrRegistry *registry) {
  for (int i = 0; i < registry->count; i++) {
    if ((registry->masks[i] &
         (COMPONENT_POSITION | COMPONENT_VELOCITY |
          COMPONENT_PLAYER_CONTROLLER | COMPONENT_MOVEMENT_INPUT)) ==
        (COMPONENT_POSITION | COMPONENT_VELOCITY | COMPONENT_PLAYER_CONTROLLER |
         COMPONENT_MOVEMENT_INPUT)) {
      BrMovementInput *input = &registry->movement_inputs[i];
      int horizontal = 0;
      float speed = registry->player_controllers[i].move_speed;

      if (input->left_pressed && !input->right_pressed)
        horizontal = -1;
      else if (input->right_pressed && !input->left_pressed)
        horizontal = 1;
      else
        horizontal = 0;

      BR_LOG_TRACE("Horizontal: %d", horizontal);
      registry->velocities[i].dx = horizontal * speed;
    }
  }
}
