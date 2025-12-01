#include "borka_ecs.h"
#include "borka_log.h"

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
