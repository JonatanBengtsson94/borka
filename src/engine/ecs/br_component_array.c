#include "br_component_array.h"
#include "borka_data_structure.h"
#include "borka_log.h"
#include <assert.h>

static void cleanup(BrComponentArray *array) {
  if (array) {
    if (array->components.data)
      br_dynamic_array_free(&array->components);
    if (array->entity_ids.data)
      br_dynamic_array_free(&array->entity_ids);
    if (array->entity_to_index)
      free(array->entity_to_index);
    free(array);
  }
}

BrComponentArray *br_component_array_create(size_t element_size) {
  BrComponentArray *array = malloc(sizeof(BrComponentArray));
  if (!array) {
    BR_LOG_ERROR("Failed to allocate component array");
    goto error;
  }

  BrDynamicArray components;
  if (!br_dynamic_array_init(&components, element_size, 1)) {
    BR_LOG_ERROR("Failed to init dynamic component array");
    goto error;
  }

  BrDynamicArray entity_ids;
  if (!br_dynamic_array_init(&entity_ids, sizeof(BrEntity), 1)) {
    BR_LOG_ERROR("Failed to init dynamic component array");
    goto error;
  }

  int *entity_map = malloc(sizeof(int) * MAX_ENTITIES);
  if (!entity_map) {
    BR_LOG_ERROR("Failed to allocate entity to index array");
    goto error;
  }

  for (int i = 0; i < MAX_ENTITIES; i++) {
    entity_map[i] = -1;
  }

  array->entity_to_index = entity_map;
  array->components = components;
  array->entity_ids = entity_ids;

  return array;

error:
  cleanup(array);
  return NULL;
}

void br_component_array_destroy(BrComponentArray *array) { cleanup(array); }

bool br_component_array_add(BrComponentArray *array, BrEntity entity,
                            const void *component) {
  assert(array);
  assert(component);
  assert(entity <= MAX_ENTITIES);

  int index = array->entity_to_index[entity];
  if (index != -1) {
    BR_LOG_WARN("Entity already has component");
    return false;
  }

  if (!br_dynamic_array_add(&array->components, component)) {
    BR_LOG_ERROR("Failed to add component to components array");
    return false;
  }

  if (!br_dynamic_array_add(&array->entity_ids, &entity)) {
    BR_LOG_ERROR("Failed to add entity to entity ids array");
    return false;
  }

  array->entity_to_index[entity] = array->entity_ids.length - 1;

  return true;
}

void *br_component_array_get(BrComponentArray *array, BrEntity entity) {
  assert(array);
  assert(entity <= MAX_ENTITIES);

  int index = array->entity_to_index[entity];
  if (index < 0) {
    BR_LOG_WARN("Could not find component for entity: %u", entity);
    return NULL;
  }

  return (char *)array->components.data +
         index * array->components.element_size;
}

bool br_component_array_remove(BrComponentArray *array, BrEntity entity) {
  assert(array);
  assert(entity <= MAX_ENTITIES);

  int index = array->entity_to_index[entity];
  if (index == -1) {
    BR_LOG_WARN(
        "Could not remove component, entity has no component of that type");
    return false;
  }

  int last_index = array->entity_ids.length - 1;
  BrEntity last_entity = ((BrEntity *)array->entity_ids.data)[last_index];

  if (!br_dynamic_array_remove(&array->components, index)) {
    BR_LOG_ERROR("Failed to remove component from components array");
    return false;
  }

  if (!br_dynamic_array_remove(&array->entity_ids, index)) {
    BR_LOG_ERROR("Failed to remove entity from entity ids array");
    return false;
  }

  if (index != last_index) {
    array->entity_to_index[last_entity] = index;
  }

  array->entity_to_index[entity] = -1;

  return true;
}
