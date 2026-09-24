#include "components/components.h"
#include "systems.h"

void system_render(BrRegistry *registry, BrRenderer *renderer,
                   const Scene *scene, const Camera *camera) {
  assert(registry);
  assert(renderer);
  assert(scene);
  assert(camera);

  // The camera moves the world. The background stays put, since moving it
  // would show its edges, and so does the UI.
  br_renderer_set_layer_offset(renderer, RENDER_LAYER_BEHIND, camera->offset);
  br_renderer_set_layer_offset(renderer, RENDER_LAYER_WORLD, camera->offset);

  br_renderer_clear(renderer, 0xFF000000);
  if (scene->background)
    br_renderer_draw_texture(renderer, RENDER_LAYER_BACKGROUND, (BrVec2){0, 0},
                             scene->background);

  BrQuery *query = br_query_begin(registry, SYSTEM_RENDER);
  while (br_query_next(query)) {
    Renderable *r =
        (Renderable *)br_query_get_component(query, COMPONENT_RENDERABLE);
    Position *p = (Position *)br_query_get_component(query, COMPONENT_POSITION);

    assert(r);
    assert(p);
    BrVec2 int_pos = {(int)p->x + r->offset.x, (int)p->y + r->offset.y};

    switch (r->type) {
    case RENDERABLE_TEXTURE:
      br_renderer_draw_texture(renderer, r->layer, int_pos, r->texture.texture);
      break;

    case RENDERABLE_TEXTURE_REGION:
      br_renderer_draw_texture_region(renderer, r->layer, int_pos,
                                      r->region.region);
      break;

    case RENDERABLE_RECTANGLE:
      if (r->rectangle.filled)
        br_renderer_draw_rectangle_filled(renderer, r->layer, int_pos,
                                          r->rectangle.size, r->rectangle.color);
      else
        br_renderer_draw_rectangle_outlined(renderer, r->layer, int_pos,
                                            r->rectangle.size,
                                            r->rectangle.color);
      break;

    case RENDERABLE_TEXT:
      assert(r->text.font && r->text.text);
      br_renderer_draw_text(renderer, r->layer, r->text.font, r->text.text,
                            int_pos);
      break;

    case RENDERABLE_FILLED_TRIANGLE:
      break;
    }
  }
  br_renderer_present(renderer);
}
