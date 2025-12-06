#include "borka.h"
#include "components/components.h"
#include "systems.h"

typedef struct {
  BrEntity entity;
  Position *pos;
  Collider *col;
} CollidingEntity;

static bool check_aabb(Position *posA, Collider *colA, Position *posB,
                       Collider *colB) {
  return (posA->x < posB->x + colB->width && posA->x + colA->width > posB->x &&
          posA->y < posB->y + colB->height && posA->y + colA->height > posB->y);
}

void system_collision(BrRegistry *registry) {
  CollidingEntity colliding_entities[MAX_ENTITIES];
  int count = 0;

  BrQuery *query = br_query_begin(registry, SYSTEM_COLLISION);
  while (br_query_next(query)) {
    Position *pos = br_query_get_component(query, COMPONENT_POSITION);
    Collider *col = br_query_get_component(query, COMPONENT_COLLIDER);

    assert(pos);
    assert(col);

    colliding_entities[count].entity = query->current_entity;
    colliding_entities[count].pos = pos;
    colliding_entities[count].col = col;
    count++;
  }

  for (int i = 0; i < count; ++i) {
    for (int j = i + 1; j < count; ++j) {
      if (check_aabb(colliding_entities[i].pos, colliding_entities[i].col,
                     colliding_entities[j].pos, colliding_entities[j].col))
        BR_LOG_DEBUG("Collision found");
    }
  }
}
