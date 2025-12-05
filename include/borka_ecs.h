#ifndef BR_ECS_H
#define BR_ECS_H

#include "borka_data_structure.h"
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

/**
 * @brief Central manager for the entity component system (ECS).
 */
typedef struct {
  BrComponentArray
      *component_arrays[MAX_COMPONENT_TYPES];  /**< Array of ComponentArrays. */
  BrSignature entity_signatures[MAX_ENTITIES]; /**< Component signature for each
                                                  entity ID. */
  BrSignature system_signatures[MAX_SYSTEMS];  /**< Component singature for each
                                                  system ID. */
  int free_entities[MAX_ENTITIES]; /**< Array holding entites that are not in
                                      use. */
  int free_top; /**< Index of the next free slot in the free_entites. */
  BrComponentTypeId next_component_id; /**< Next available component id. */
  BrSystemId next_system_id;           /**< Next available system id. */
} BrRegistry;

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
void *br_component_get(BrRegistry *registry, BrComponentTypeId component_type,
                       BrEntity entity);

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
 * @param required_components What components must be held for the system to
 * process a entity.
 * @param components_count The number of required components.
 * @return The ID in the registry for the system, or BR_INVALID_SYSTEM_ID on
 * failure.
 */
BrSystemId br_register_system(BrRegistry *registry,
                              BrComponentTypeId *required_components,
                              size_t components_count);

#endif // BR_ECS_H
