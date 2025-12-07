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

void system_collision_detection(BrRegistry *registry) {
  CollidingEntity colliding_entities[MAX_ENTITIES];
  int count = 0;

  BrQuery *query = br_query_begin(registry, SYSTEM_COLLISION_DETECTION);
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
      CollidingEntity a = colliding_entities[i];
      CollidingEntity b = colliding_entities[j];
      if ((a.col->mask & b.col->layer) == 0)
        continue;
      if ((b.col->mask & a.col->layer) == 0)
        continue;
      if (check_aabb(a.pos, a.col, b.pos, b.col)) {
        Collision col = {a.entity, b.entity};
        br_component_add(registry, a.entity, COMPONENT_COLLISION, &col);
        BR_LOG_TRACE("%u has collided with %u", col.entityA, col.entityB);
      }
    }
  }
}
