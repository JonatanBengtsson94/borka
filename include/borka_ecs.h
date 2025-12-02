#ifndef BR_ECS_H
#define BR_ECS_H

#include "borka_render.h"
#include "borka_texture.h"
#include <stdint.h>

/**
 * @brief The maximum number of entities the registry can hold.
 */
#define MAX_ENTITES 100

/**
 * @brief Bitmask flags representing the presence of a component type on an
 * Entity.
 */
typedef enum {
  COMPONENT_NONE = 0,
  COMPONENT_POSITION = 1 << 0,
  COMPONENT_VELOCITY = 1 << 1,
  COMPONENT_SPRITE = 1 << 2,
  COMPONENT_PLAYER_CONTROLLER = 1 << 3,
} BrComponentMask;

/**
 * @brief Holds the position of an Entity in the game world.
 */
typedef struct {
  float x; /**< X-coordinate */
  float y; /**< Y-coordinate */
} BrPosition;

/**
 * @brief Holds the velocity (change in position per second) of an Entity.
 */
typedef struct {
  float dx; /**< Velocity along the X-axis. */
  float dy; /**< Velocity along the Y-axis. */
} BrVelocity;

/**
 * @brief Holds the visual asset data for rendering an Entity.
 */
typedef struct {
  BrTexture *texture; /**< Pointer to the texture instance. */
} BrSprite;

/**
 * @brief Stores the data for player movement.
 */
typedef struct {
  float move_speed;    /**< The maximum movement speed (pixels per second). */
  int horizontal_axis; /**< Movement across the x-axis (-1 left, 0 none, 1
                          right). */
} BrPlayerController;

/**
 * @brief The central database for the Entity Component System.
 */
typedef struct {
  int count; /**< The number of currently active entites */
  BrComponentMask masks[MAX_ENTITES]; /** The component mask (signature) for
                                         each entity ID. */
  BrPosition
      positions[MAX_ENTITES]; /** Array storing all Position components. */
  BrVelocity
      velocities[MAX_ENTITES];   /** Array storing all Velocity components. */
  BrSprite sprites[MAX_ENTITES]; /** Array starting all Sprite components. */
  BrPlayerController
      player_controllers[MAX_ENTITES]; /** Array storing all player controller
                                          components. */
} BrRegistry;

/**
 * @brief Unique identifier (ID) and handle for an Entity withing the Registry.
 */
typedef uint32_t BrEntity;

/**
 * @brief Initializes and activates a new Entity in the registry.
 *
 * @param registry The central ECS data store.
 * @return BrEntity The unique ID (index) assigned to the newly created entity.
 * Returns an invalid ID (UINT32_MAX) on failure.
 */
BrEntity create_entity(BrRegistry *registry);

/**
 * @brief System responsible for updating Entity position based on their
 * velocity.
 *
 * @param registry The central ECS data store.
 * @param delta_time The time elapsed since last frame (in seconds).
 */
void system_movement(BrRegistry *registry, double delta_time);

/**
 * @brief System responsible for rendering all visible Entities to the screen.
 *
 * @param registry The central ECS data store.
 * @param app The main application struct.
 */
void system_render(BrRegistry *registry, BrRenderer *renderer);

/**
 * @brief System responsible for controlling the player entity.
 *
 * @param registry The central ECS data store.
 */
void system_player_controller(BrRegistry *registry);

#endif // BR_ECS_H
