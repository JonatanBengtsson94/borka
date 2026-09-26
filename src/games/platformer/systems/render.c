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
    BrVec2 int_pos = {(int)p->x, (int)p->y};

    switch (r->type) {
    case RENDERABLE_TEXTURE:
      br_renderer_draw_texture(renderer, r->layer, int_pos, r->texture.texture);
      break;

    case RENDERABLE_TEXTURE_REGION:
      br_renderer_draw_texture_region(renderer, r->layer, int_pos,
                                      r->region.region);
      break;

    case RENDERABLE_TEXT:
      assert(r->text.font && r->text.text);
      br_renderer_draw_text(renderer, r->layer, r->text.font, r->text.text,
                            int_pos);
      break;
    }
  }
  br_renderer_present(renderer);
}
