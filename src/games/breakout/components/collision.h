#include "borka.h"

#define MAX_COLLISIONS_PER_ENTITY 8

typedef struct {
  BrEntity colliding_entities[MAX_COLLISIONS_PER_ENTITY];
  uint8_t count;
} Collision;
