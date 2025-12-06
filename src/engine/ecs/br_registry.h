#ifndef BR_REGISTRY_H
#define BR_REGISTRY_H

#include "borka_ecs.h"

BrRegistry *br_registry_create();
void br_registry_destroy(BrRegistry *registry);

#endif // BR_REGISTRY_H
