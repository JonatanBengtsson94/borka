#include "components/components.h"
#include "systems.h"

void system_animation(BrRegistry *registry, double delta_time) {
  assert(registry);

  BrQuery *query = br_query_begin(registry, SYSTEM_ANIMATION);
  while (br_query_next(query)) {
    Animator *a = (Animator *)br_query_get_component(query, COMPONENT_ANIMATOR);
    Renderable *r =
        (Renderable *)br_query_get_component(query, COMPONENT_RENDERABLE);
    assert(r);
    assert(a);
    assert(a->frames);
    assert(a->number_of_frames > 0);
    assert(a->frame_time > 0);

    a->elapsed_time += (float)delta_time;

    while (a->elapsed_time >= a->frame_time) {
      a->elapsed_time -= a->frame_time;
      a->current_frame++;

      if (a->current_frame >= a->number_of_frames) {
        if (a->loop) {
          a->current_frame = 0;
        } else {
          a->current_frame = a->number_of_frames - 1;
          a->finished = true;
          break;
        }
      }
    }
    r->region.region = a->frames[a->current_frame];

    if (!a->loop && a->finished) {
      br_entity_destroy(registry, query->current_entity);
      BR_LOG_TRACE("Destroying animation entity %d", query->current_entity);
    }
  }
}
