#include "br_registry.h"
#include "borka_log.h"
#include "br_component_array.h"
#include <stdbool.h>

BrRegistry *br_registry_create() {
  BrRegistry *registry = calloc(1, sizeof(BrRegistry));
  if (!registry) {
    BR_LOG_ERROR("Failed to allocate registry");
    return NULL;
  }

  for (int i = 0; i < MAX_ENTITIES; i++) {
    registry->free_entities[i] = i;
  }
  registry->free_top = MAX_ENTITIES - 1;

  BR_LOG_INFO("ECS Registry initialized");
  return registry;
}

void br_registry_destroy(BrRegistry *registry) {
  if (!registry)
    return;
  for (int i = 0; i < MAX_COMPONENT_TYPES; i++) {
    if (registry->component_arrays[i])
      br_component_array_destroy(registry->component_arrays[i]);
  }
  free(registry);
}

BrEntity br_entity_create(BrRegistry *registry) {
  if (!registry) {
    BR_LOG_ERROR("Could not create entity, registry is NULL");
    return BR_INVALID_ENTITY;
  }

  if (registry->free_top < 0) {
    BR_LOG_ERROR("Too many entities in registry");
    return BR_INVALID_ENTITY;
  }

  BrEntity entity = registry->free_entities[--registry->free_top];
  registry->entity_signatures[entity] = 0;

  return entity;
}

void br_entity_destroy(BrRegistry *registry, BrEntity entity) {
  if (entity >= MAX_ENTITIES)
    return;

  BrSignature signature = registry->entity_signatures[entity];
  for (int i = 0; i < MAX_COMPONENT_TYPES; i++) {
    if (signature & (1 << i)) {
      br_component_remove(registry, entity, i);
    }
  }

  registry->entity_signatures[entity] = 0;
  registry->free_entities[registry->free_top++] = entity;
}

BrComponentTypeId br_register_component(BrRegistry *registry,
                                        size_t component_size) {
  if (!registry) {
    BR_LOG_ERROR("Could not register component, registry is NULL");
    return BR_INVALID_COMPONENT_TYPE;
  }

  if (registry->next_component_id >= MAX_COMPONENT_TYPES) {
    BR_LOG_ERROR("Too many component type registred (Max: %u)",
                 MAX_COMPONENT_TYPES);
    return BR_INVALID_COMPONENT_TYPE;
  }

  BrComponentArray *array = br_component_array_create(component_size);
  if (!array) {
    BR_LOG_ERROR(
        "Could not register component, failed to create component array");
    return BR_INVALID_COMPONENT_TYPE;
  }

  BrComponentTypeId new_id = registry->next_component_id;
  registry->component_arrays[new_id] = array;
  registry->next_component_id++;

  return new_id;
}

bool br_component_add(BrRegistry *registry, BrEntity entity,
                      BrComponentTypeId component_type, const void *component) {
  if (!registry) {
    BR_LOG_ERROR("Could not add component, registry is NULL");
    return false;
  }

  if (entity >= MAX_ENTITIES) {
    BR_LOG_ERROR("Could not add component, invalid entity");
    return false;
  }

  if (component_type >= MAX_COMPONENT_TYPES) {
    BR_LOG_ERROR("Could not add component, invalid component id");
    return false;
  }

  BrComponentArray *component_array =
      registry->component_arrays[component_type];

  if (!component_array) {
    BR_LOG_ERROR("Could not add component, array is NULL");
    return false;
  }

  if (!br_component_array_add(component_array, entity, component)) {
    BR_LOG_ERROR("Could not add component, failed to add to component array");
    return false;
  }

  registry->entity_signatures[entity] |= (1 << component_type);

  return true;
}

void *br_component_get(const BrRegistry *registry,
                       BrComponentTypeId component_type, BrEntity entity) {
  if (!registry) {
    BR_LOG_ERROR("Could not get component, registry is NULL");
    return NULL;
  }

  if (component_type >= MAX_COMPONENT_TYPES) {
    BR_LOG_ERROR("Could not get component, invalid component id");
    return NULL;
  }

  if (entity >= MAX_ENTITIES) {
    BR_LOG_ERROR("Could not get component, invalid entity");
    return NULL;
  }

  BrComponentArray *component_array =
      registry->component_arrays[component_type];
  if (!component_array) {
    BR_LOG_ERROR("Could not get component, component array is NULL");
    return NULL;
  }

  return br_component_array_get(component_array, entity);
}

void br_component_remove(BrRegistry *registry, BrEntity entity,
                         BrComponentTypeId component_type) {
  if (!registry) {
    BR_LOG_ERROR("Could not remove component, registry is NULL");
    return;
  }

  if (component_type >= MAX_COMPONENT_TYPES) {
    BR_LOG_ERROR("Could not remove component, invalid component id");
    return;
  }

  if (entity >= MAX_ENTITIES) {
    BR_LOG_ERROR("Could not remove component, invalid entity");
    return;
  }

  BrComponentArray *component_array =
      registry->component_arrays[component_type];
  if (!component_array) {
    BR_LOG_ERROR("Could not remove component. Component type has no array");
    return;
  }

  if (!br_component_array_remove(component_array, entity))
    BR_LOG_ERROR("Failed to remove component");

  return;
}

BrSystemId br_register_system(BrRegistry *registry,
                              BrComponentTypeId primary_component,
                              BrComponentTypeId *required_components,
                              size_t components_count) {
  if (!registry) {
    BR_LOG_ERROR("Could not register system, registry is NULL");
    return BR_INVALID_SYSTEM_ID;
  }
  if (registry->next_system_id >= MAX_SYSTEMS) {
    BR_LOG_ERROR(
        "Could not register system, maximum number of systems reached");
    return BR_INVALID_SYSTEM_ID;
  }

  BrSignature system_signature = 0;
  for (size_t i = 0; i < components_count; i++) {
    BrComponentTypeId component_id = required_components[i];
    if (component_id >= MAX_COMPONENT_TYPES) {
      BR_LOG_ERROR(
          "Could not register system, required component has invalid id");
      return BR_INVALID_SYSTEM_ID;
    }
    system_signature |= (1 << component_id);
  }

  BrSystemId new_id = registry->next_system_id++;
  registry->system_signatures[new_id] = system_signature;

  BrQuery *query = &registry->system_queries[new_id];
  query->system_id = new_id;
  query->registry = registry;
  query->primary_array = registry->component_arrays[primary_component];
  query->current_index = 0;
  query->current_entity = BR_INVALID_ENTITY;

  return new_id;
}
