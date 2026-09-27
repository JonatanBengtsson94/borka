#ifndef BR_ECS_H
#define BR_ECS_H

#include "borka_data_structure.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Maximum number of entities the registry can hold.
 */
#define MAX_ENTITIES 100

/**
 * @brief Maximum number of distinct component types that can be registered.
 */
#define MAX_COMPONENT_TYPES 15

/**
 * @brief Maximum number of systems the registry can hold.
 */
#define MAX_SYSTEMS 15

/**
 * @brief Invalid entity ID constant.
 */
#define BR_INVALID_ENTITY UINT32_MAX

/**
 * @brief Invalid component type ID constant.
 */
#define BR_INVALID_COMPONENT_TYPE UINT16_MAX

/**
 * @brief Invalid system ID constant.
 */
#define BR_INVALID_SYSTEM_ID UINT8_MAX

/**
 * @brief Unique identifier (ID) and handle for an Entity within the Registry.
 */
typedef uint32_t BrEntity;

/**
 * @brief Unique identifier for a component type.
 */
typedef uint16_t BrComponentTypeId;

/**
 * @brief Unique identifier for a component type.
 */
typedef uint8_t BrSystemId;

/**
 * @brief Bitset representing an entitys component composition.
 */
typedef uint16_t BrSignature;

/**
 * @brief Container for all instances of a specific component type.
 */
typedef struct {
  BrDynamicArray components; /**< Array of components. */
  BrDynamicArray entity_ids; /**< Array of entity ids owning each component. */
  int *entity_to_index; /**< Maps entity to index in the component array. */
} BrComponentArray;

typedef struct BrRegistry BrRegistry;

/**
 * @brief Represents an active iteration over entities relevant to a system.
 */
typedef struct {
  BrRegistry *registry;            /**< Central ECS data store. */
  BrComponentArray *primary_array; /**< Primary component array to iterate over
                                      (largest one). */
  size_t current_index;            /**< Current index in the primary array. */
  BrEntity current_entity;         /**< Entity ID at the current index. */
  BrSystemId system_id;            /**< ID of the system being queried. */
} BrQuery;

/**
 * @brief Central manager for the entity component system (ECS).
 */
struct BrRegistry {
  BrComponentArray
      *component_arrays[MAX_COMPONENT_TYPES];  /**< Array of ComponentArrays. */
  BrQuery system_queries[MAX_SYSTEMS];         /**< Array holding all queries
                                                  for registered systems. */
  BrSignature entity_signatures[MAX_ENTITIES]; /**< Component signature for each
                                                  entity ID. */
  BrSignature system_signatures[MAX_SYSTEMS];  /**< Component singature for each
                                                  system ID. */
  int free_entities[MAX_ENTITIES]; /**< Array holding entites that are not in
                                      use. */
  int free_top; /**< Number of free entities; the top of the free_entities
                   stack is free_entities[free_top - 1]. */
  bool alive[MAX_ENTITIES]; /**< Whether each entity id is currently in use. */
  BrComponentTypeId next_component_id; /**< Next available component id. */
  BrSystemId next_system_id;           /**< Next available system id. */
};

/**
 * @brief Creates a new Entity in the registry.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @return BrEntity Unique ID (index) assigned to the newly created entity,
 * or BR_INVALID_ENTITY if all MAX_ENTITIES entities are in use.
 *
 * @note Should be destroyed with destroy_entity() when no longer in use.
 */
BrEntity br_entity_create(BrRegistry *registry);

/**
 * @brief Removes an entity from the registry.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param entity Entity that should be destroyed. Must be alive.
 */
void br_entity_destroy(BrRegistry *registry, BrEntity entity);

/**
 * @brief Checks whether an entity id is currently in use.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param entity Entity to check. BR_INVALID_ENTITY is allowed and reports
 * false.
 * @return True if the entity is alive, false otherwise.
 */
bool br_entity_is_alive(const BrRegistry *registry, BrEntity entity);

/**
 * @brief Registers a new component type with the registry.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param component_size Size of a single component instance in bytes. Must
 * be greater than 0.
 * @return The ID in the registry for the component type, or
 * BR_INVALID_COMPONENT_TYPE if MAX_COMPONENT_TYPES are already registered or
 * memory allocation fails.
 *
 * @note Component types should typically be registered once at startup.
 */
BrComponentTypeId br_register_component(BrRegistry *registry,
                                        size_t component_size);

/**
 * @brief Attaches a component instanced to a specific entity.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param entity Entity that the component belongs to. Must be a valid entity.
 * @param component_type Unique ID of a registered component type.
 * @param component A constant pointer to the data of the component to be
 * copied. Must not be NULL.
 * @return True on success, false if the entity already has the component or
 * memory allocation fails.
 */
bool br_component_add(BrRegistry *registry, BrEntity entity,
                      BrComponentTypeId component_type, const void *component);

/**
 * @brief Get a pointer to the entities component data.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param component_type Unique ID of a registered component type.
 * @param entity The entity whose component data are requested. Must have a
 * component of component_type; check with br_component_exists() first if
 * unsure.
 * @return Pointer to the component data. Never NULL.
 */
void *br_component_get(const BrRegistry *registry,
                       BrComponentTypeId component_type, BrEntity entity);

/**
 * @brief Removes a component from an entity.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param entity Entity that should have its component removed. Must have a
 * component of component_type; check with br_component_exists() first if
 * unsure.
 * @param component_type Unique ID of a registered component type.
 */
void br_component_remove(BrRegistry *registry, BrEntity entity,
                         BrComponentTypeId component_type);
/**
 * @brief Check if a component type exists on the entity.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param entity Entity that should have its component checked. Must be a
 * valid entity.
 * @param component_type Component type ID of the component that should be
 * checked. Must be less than MAX_COMPONENT_TYPES.
 * @return True if the component exists, false otherwise.
 */
bool br_component_exists(BrRegistry *registry, BrEntity entity,
                         BrComponentTypeId component_type);

/**
 * @brief Registers a new system with the registry.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param primary_component The most unique type of the component for the
 * system. Will be used for iteration in query. Must be registered and be one
 * of required_components.
 * @param required_components What components must be held for the system to
 * process a entity. Each must be less than MAX_COMPONENT_TYPES. May only be
 * NULL when components_count is 0.
 * @param components_count The number of required components.
 * @return The ID in the registry for the system, or BR_INVALID_SYSTEM_ID if
 * MAX_SYSTEMS are already registered.
 */
BrSystemId br_register_system(BrRegistry *registry,
                              BrComponentTypeId primary_component,
                              BrComponentTypeId *required_components,
                              size_t components_count);

/**
 * @brief Starts a new query for system iteration.
 *
 * @param registry Central ECS data store. Must not be NULL.
 * @param system_id The ID of a registered system.
 * @return The system's BrQuery, reset to the start. Never NULL.
 */
static inline BrQuery *br_query_begin(BrRegistry *registry,
                                      BrSystemId system_id) {
  assert(registry);
  assert(system_id < MAX_SYSTEMS);

  BrQuery *query = &registry->system_queries[system_id];
  assert(query);
  assert(query->primary_array);

  query->current_index = 0;
  query->current_entity = BR_INVALID_ENTITY;

  return query;
}

/**
 * @brief Advances the iterator to the next matching entity.
 *
 * @param query Pointer to the query instance to advance. Must not be NULL.
 * @return True if a new entity was found, false if iteration is complete.
 */
static inline bool br_query_next(BrQuery *query) {
  assert(query);
  assert(query->primary_array);

  BrComponentArray *primary = query->primary_array;
  BrRegistry *registry = query->registry;
  BrSignature system_signature = registry->system_signatures[query->system_id];
  size_t count = primary->entity_ids.length;

  while (query->current_index < count) {
    BrEntity entity =
        ((BrEntity *)primary->entity_ids.data)[query->current_index];
    if ((registry->entity_signatures[entity] & system_signature) ==
        system_signature) {
      query->current_entity = entity;
      query->current_index++;
      return true;
    }

    query->current_index++;
  }

  query->current_entity = BR_INVALID_ENTITY;
  return false;
}

/**
 * @brief Retrieves the component data for the current entity in the query.
 *
 * @param query Active query, positioned on an entity by br_query_next().
 * @param component_type A registered component type that the system
 * requires.
 * @return Pointer to the component data. Never NULL.
 */
static inline void *br_query_get_component(const BrQuery *query,
                                           BrComponentTypeId component_type) {
  assert(query);
  assert(query->current_entity < MAX_ENTITIES);
  assert(component_type < MAX_COMPONENT_TYPES);

  BrComponentArray *array = query->registry->component_arrays[component_type];
  assert(array);
  size_t component_size = array->components.element_size;

  if (array == query->primary_array) {
    size_t index = query->current_index - 1;
    return (char *)array->components.data + index * component_size;
  }

  BrEntity entity = query->current_entity;
  int component_index = array->entity_to_index[entity];
  // Only components the system requires are guaranteed to be present.
  assert(component_index >= 0);

  return (char *)array->components.data + component_index * component_size;
}

#endif // BR_ECS_H
