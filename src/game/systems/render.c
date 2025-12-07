#include "components/components.h"
#include "systems.h"

void system_render(BrRegistry *registry, BrRenderer *renderer) {
  assert(registry);
  assert(renderer);

  br_renderer_clear(renderer, 0xFF000000);

  BrQuery *query = br_query_begin(registry, SYSTEM_RENDER);
  while (br_query_next(query)) {
    Renderable *r =
        (Renderable *)br_query_get_component(query, COMPONENT_RENDERABLE);
    Position *p = (Position *)br_query_get_component(query, COMPONENT_POSITION);

    assert(r);
    assert(p);
    BrVec2 int_pos = {p->x, p->y};

    switch (r->type) {
    case RENDERABLE_SPRITE:
      assert(r->sprite.texture);
      br_renderer_draw_texture(renderer, int_pos, r->sprite.texture);
      break;

    case RENDERABLE_RECTANGLE:
      if (r->rectangle.filled)
        br_renderer_draw_rectangle_filled(renderer, int_pos, r->rectangle.size,
                                          r->rectangle.color);
      else
        br_renderer_draw_rectangle_outlined(
            renderer, int_pos, r->rectangle.size, r->rectangle.color);
      break;

    case RENDERABLE_FILLED_TRIANGLE:
      break;
    }
    br_renderer_present(renderer);
  }
}
