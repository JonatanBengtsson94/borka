#ifndef BR_COMPONENT_ARRAY_H
#define BR_COMPONENT_ARRAY_H

#include "borka_ecs.h"

BrComponentArray *br_component_array_create(size_t element_size);
void br_component_array_destroy(BrComponentArray *array);
bool br_component_array_add(BrComponentArray *array, BrEntity entity,
                            const void *component);
void *br_component_array_get(BrComponentArray *array, BrEntity entity);
bool br_component_array_remove(BrComponentArray *array, BrEntity entity);

#endif // BR_COMPONENT_ARRAY_H
