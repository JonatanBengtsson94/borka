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
  int free_top; /**< Index of the next free slot in the free_entites. */
  BrComponentTypeId next_component_id; /**< Next available component id. */
  BrSystemId next_system_id;           /**< Next available system id. */
};

/**
 * @brief Creates a new Entity in the registry.
 *
 * @param registry Central ECS data store.
 * @return BrEntity Unique ID (index) assigned to the newly created entity,
 * or BR_INVALID_ENTITY if creation fails.
 *
 * @note Should be destroyed with destroy_entity() when no longer in use.
 */
BrEntity br_entity_create(BrRegistry *registry);

/**
 * @brief Removes an entity from the registry.
 *
 * @param registry Central ECS data store.
 * @param entity Entity that should be destroyed.
 */
void br_entity_destroy(BrRegistry *registry, BrEntity entity);

/**
 * @brief Registers a new component type with the registry.
 *
 * @param registry Central ECS data store.
 * @param component_size Size of a single component instance in bytes.
 * @return The ID in the registry for the component type, or
 * BR_INVALID_COMPONENT_TYPE on failure.
 *
 * @note Component types should typically be registered once at startup.
 */
BrComponentTypeId br_register_component(BrRegistry *registry,
                                        size_t component_size);

/**
 * @brief Attaches a component instanced to a specific entity.
 *
 * @param registry Central ECS data store.
 * @param entity Entity that the component belongs to.
 * @param component_type Unique ID of the component type being added.
 * @param component A constant pointer to the data of the component to be
 * copied.
 * @return True on success, false if fails.
 */
bool br_component_add(BrRegistry *registry, BrEntity entity,
                      BrComponentTypeId component_type, const void *component);

/**
 * @brief Get a pointer to the entities component data.
 *
 * @param registry Central ECS data store.
 * @param component_type The type of component to fetch.
 * @param entity The entity whose component data are requested.
 */
void *br_component_get(const BrRegistry *registry,
                       BrComponentTypeId component_type, BrEntity entity);

/**
 * @brief Removes a component from an entity.
 *
 * @param registry Central ECS data store.
 * @param entity Entity that should have its component removed.
 * @param component_type Component type ID of the component that should be
 * removed.
 */
void br_component_remove(BrRegistry *registry, BrEntity entity,
                         BrComponentTypeId component_type);

/**
 * @brief Registers a new system with the registry.
 *
 * @param registry Central ECS data store.
 * @param primary_component The most unique type of the component for the
 * system. Will be used for iteration in query.
 * @param required_components What components must be held for the system to
 * process a entity.
 * @param components_count The number of required components.
 * @return The ID in the registry for the system, or BR_INVALID_SYSTEM_ID on
 * failure.
 */
BrSystemId br_register_system(BrRegistry *registry,
                              BrComponentTypeId primary_component,
                              BrComponentTypeId *required_components,
                              size_t components_count);

/**
 * @brief Starts a new query for system iteration.
 *
 * @param registry Central ECS data store.
 * @param system_id The ID of the system to query.
 * @return The new BrQuery instance, or NULL on failure.
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
 * @param query Pointer to the query instance to advance.
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
 * @param query Active query.
 * @param component_type Type of component to get.
 * @return Pointer to the component data, or NULL on failure.
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

  return (char *)array->components.data + component_index * component_size;
}

#endif // BR_ECS_H
