#include "borka.h"
#include "borka_ecs.h"
#include "components/components.h"
#include "systems.h"

typedef struct {
  BrEntity entity;
  Position *pos;
  Collider *col;
} CollidingEntity;

static bool check_aabb(Position *posA, Collider *colA, Position *posB,
                       Collider *colB) {
  return (posA->x < posB->x + colB->size.x &&
          posA->x + colA->size.x > posB->x &&
          posA->y < posB->y + colB->size.y && posA->y + colA->size.y > posB->y);
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
        if (!br_component_exists(registry, a.entity, COMPONENT_COLLISION)) {
          Collision new_collision = {0};
          new_collision.colliding_entities[0] = b.entity;
          new_collision.count++;
          br_component_add(registry, a.entity, COMPONENT_COLLISION,
                           &new_collision);
        } else {
          Collision *collision =
              br_component_get(registry, COMPONENT_COLLISION, a.entity);
          assert(collision);
          if (collision->count < MAX_COLLISIONS_PER_ENTITY) {
            collision->colliding_entities[collision->count++] = b.entity;
          } else {
            BR_LOG_WARN(
                "Maximum number of collision reached, dropping collision");
          }
        }
        BR_LOG_TRACE("%u has collided with %u", a.entity, b.entity);
      }
    }
  }
}
