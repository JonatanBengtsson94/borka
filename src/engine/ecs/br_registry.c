#include "br_registry.h"
#include "borka_ecs.h"
#include "borka_log.h"
#include "br_component_array.h"

BrRegistry *br_registry_create() {
  BrRegistry *registry = calloc(1, sizeof(BrRegistry));
  if (!registry) {
    BR_LOG_ERROR("Failed to allocate registry");
    return NULL;
  }

  for (int i = 0; i < MAX_ENTITIES; i++) {
    registry->free_entities[i] = i;
  }
  registry->free_top = MAX_ENTITIES;

  BR_LOG_DEBUG("ECS Registry initialized");
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
  assert(registry);

  if (registry->free_top <= 0) {
    BR_LOG_ERROR("Too many entities in registry (Max: %d)", MAX_ENTITIES);
    return BR_INVALID_ENTITY;
  }

  BrEntity entity = registry->free_entities[--registry->free_top];
  registry->entity_signatures[entity] = 0;
  registry->alive[entity] = true;

  BR_LOG_TRACE("Created entity %u (%d free)", entity, registry->free_top);
  return entity;
}

void br_entity_destroy(BrRegistry *registry, BrEntity entity) {
  assert(registry);
  assert(entity < MAX_ENTITIES);
  // Destroying twice would push the id onto the free stack twice.
  assert(registry->alive[entity]);

  BrSignature signature = registry->entity_signatures[entity];
  for (int i = 0; i < MAX_COMPONENT_TYPES; i++) {
    if (signature & (1 << i)) {
      br_component_remove(registry, entity, i);
    }
  }

  registry->entity_signatures[entity] = 0;
  registry->alive[entity] = false;
  registry->free_entities[registry->free_top++] = entity;
  BR_LOG_TRACE("Destroyed entity %u (%d free)", entity, registry->free_top);
}

bool br_entity_is_alive(const BrRegistry *registry, BrEntity entity) {
  assert(registry);

  // Asking about BR_INVALID_ENTITY is a valid question with a known answer.
  if (entity >= MAX_ENTITIES)
    return false;

  return registry->alive[entity];
}

BrComponentTypeId br_register_component(BrRegistry *registry,
                                        size_t component_size) {
  assert(registry);
  assert(component_size > 0);

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

  BR_LOG_DEBUG("Registered component type %u (%zu bytes)", new_id,
               component_size);
  return new_id;
}

bool br_component_add(BrRegistry *registry, BrEntity entity,
                      BrComponentTypeId component_type, const void *component) {
  assert(registry);
  assert(component);
  assert(entity < MAX_ENTITIES);
  assert(component_type < MAX_COMPONENT_TYPES);

  BrComponentArray *component_array =
      registry->component_arrays[component_type];
  // NULL means the component type was never registered.
  assert(component_array);

  if (!br_component_array_add(component_array, entity, component)) {
    BR_LOG_ERROR("Could not add component, failed to add to component array");
    return false;
  }

  registry->entity_signatures[entity] |= (1 << component_type);

  return true;
}

void *br_component_get(const BrRegistry *registry,
                       BrComponentTypeId component_type, BrEntity entity) {
  assert(registry);
  assert(component_type < MAX_COMPONENT_TYPES);
  assert(entity < MAX_ENTITIES);

  BrComponentArray *component_array =
      registry->component_arrays[component_type];
  // NULL means the component type was never registered.
  assert(component_array);

  return br_component_array_get(component_array, entity);
}

void br_component_remove(BrRegistry *registry, BrEntity entity,
                         BrComponentTypeId component_type) {
  assert(registry);
  assert(component_type < MAX_COMPONENT_TYPES);
  assert(entity < MAX_ENTITIES);

  BrComponentArray *component_array =
      registry->component_arrays[component_type];
  // NULL means the component type was never registered.
  assert(component_array);

  br_component_array_remove(component_array, entity);
  registry->entity_signatures[entity] &= ~(1 << component_type);
  BR_LOG_TRACE("Removed component type %u from entity %u", component_type,
               entity);
}

bool br_component_exists(BrRegistry *registry, BrEntity entity,
                         BrComponentTypeId component_type) {
  assert(registry);
  assert(entity < MAX_ENTITIES);
  assert(component_type < MAX_COMPONENT_TYPES);

  return (registry->entity_signatures[entity] & (1 << component_type)) != 0;
}

BrSystemId br_register_system(BrRegistry *registry,
                              BrComponentTypeId primary_component,
                              BrComponentTypeId *required_components,
                              size_t components_count) {
  assert(registry);
  assert(required_components || components_count == 0);
  assert(primary_component < MAX_COMPONENT_TYPES);
  // Queries iterate the primary component's array, so it must exist.
  assert(registry->component_arrays[primary_component]);

  if (registry->next_system_id >= MAX_SYSTEMS) {
    BR_LOG_ERROR(
        "Could not register system, maximum number of systems reached");
    return BR_INVALID_SYSTEM_ID;
  }

  BrSignature system_signature = 0;
  for (size_t i = 0; i < components_count; i++) {
    BrComponentTypeId component_id = required_components[i];
    assert(component_id < MAX_COMPONENT_TYPES);
    system_signature |= (1 << component_id);
  }
  // Iterating an array the system doesn't require would visit entities
  // that can never match.
  assert(system_signature & (1 << primary_component));

  BrSystemId new_id = registry->next_system_id++;
  registry->system_signatures[new_id] = system_signature;

  BrQuery *query = &registry->system_queries[new_id];
  query->system_id = new_id;
  query->registry = registry;
  query->primary_array = registry->component_arrays[primary_component];
  query->current_index = 0;
  query->current_entity = BR_INVALID_ENTITY;

  BR_LOG_DEBUG("Registered system %u (%zu required components)", new_id,
               components_count);
  return new_id;
}
