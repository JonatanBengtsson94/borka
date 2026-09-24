#include "components/components.h"
#include "systems.h"

typedef struct {
  BrEntity entity;
  Renderable *renderable;
  Position *position;
} DrawItem;

static bool draw_before(const DrawItem *a, const DrawItem *b) {
  if (a->renderable->layer != b->renderable->layer)
    return a->renderable->layer < b->renderable->layer;
  return a->entity < b->entity;
}

void system_render(BrRegistry *registry, BrRenderer *renderer,
                   const Scene *scene) {
  assert(registry);
  assert(renderer);
  assert(scene);

  br_renderer_clear(renderer, 0xFF000000);
  if (scene->background)
    br_renderer_draw_texture(renderer, (BrVec2){0, 0}, scene->background);

  // Storage order changes whenever a component is removed, so draw order
  // comes from sorting by layer, then entity for a stable order within one.
  DrawItem items[MAX_ENTITIES];
  int count = 0;

  BrQuery *query = br_query_begin(registry, SYSTEM_RENDER);
  while (br_query_next(query)) {
    DrawItem item = {
        .entity = query->current_entity,
        .renderable =
            (Renderable *)br_query_get_component(query, COMPONENT_RENDERABLE),
        .position =
            (Position *)br_query_get_component(query, COMPONENT_POSITION),
    };
    assert(item.renderable);
    assert(item.position);

    // Insertion sort: cheap for at most MAX_ENTITIES items.
    int i = count++;
    while (i > 0 && draw_before(&item, &items[i - 1])) {
      items[i] = items[i - 1];
      i--;
    }
    items[i] = item;
  }

  for (int i = 0; i < count; i++) {
    Renderable *r = items[i].renderable;
    Position *p = items[i].position;
    BrVec2 int_pos = {p->x, p->y};

    switch (r->type) {
    case RENDERABLE_TEXTURE:
      br_renderer_draw_texture(renderer, int_pos, r->texture.texture);
      break;

    case RENDERABLE_TEXTURE_REGION:
      br_renderer_draw_texture_region(renderer, int_pos, r->region.region);
      break;

    case RENDERABLE_RECTANGLE:
      if (r->rectangle.filled)
        br_renderer_draw_rectangle_filled(renderer, int_pos, r->rectangle.size,
                                          r->rectangle.color);
      else
        br_renderer_draw_rectangle_outlined(
            renderer, int_pos, r->rectangle.size, r->rectangle.color);
      break;

    case RENDERABLE_TEXT:
      assert(r->text.font && r->text.text);
      br_renderer_draw_text(renderer, r->text.font, r->text.text, int_pos);
      break;

    case RENDERABLE_FILLED_TRIANGLE:
      break;
    }
  }
  br_renderer_present(renderer);
}
