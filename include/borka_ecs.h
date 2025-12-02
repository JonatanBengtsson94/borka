#ifndef BR_ECS_H
#define BR_ECS_H

#include "borka_events.h"
#include "borka_render.h"
#include "borka_texture.h"
#include <stdbool.h>
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
  COMPONENT_MOVEMENT_INPUT = 1 << 4,
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
 * @brief Stores the movement input data.
 */
typedef struct {
  bool left_pressed;  /** True if left key is held down. */
  bool right_pressed; /** True if right key is held down. */
} BrMovementInput;

/**
 * @brief Stores the configuration for player-controlled movement.
 */
typedef struct {
  float move_speed; /**< The maximum movement speed (pixels per second). */
} BrPlayerController;

/**
 * @brief The central database for the Entity Component System.
 */
typedef struct {
  int count; /**< The number of currently active entites */
  BrComponentMask masks[MAX_ENTITES]; /** The component mask (signature) for
                                         each entity ID. */
  BrPosition positions[MAX_ENTITES];
  BrVelocity velocities[MAX_ENTITES];
  BrSprite sprites[MAX_ENTITES];
  BrPlayerController player_controllers[MAX_ENTITES];
  BrMovementInput movement_inputs[MAX_ENTITES];
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
 * @brief System responsible for rendering all visible Entities to the screen.
 *
 * @param registry The central ECS data store.
 * @param app The main application struct.
 */
void system_render(BrRegistry *registry, BrRenderer *renderer);

/**
 * @brief System responsible for updating all entities based on input events.
 *
 * @param registry The central ECS data store.
 * @param e The event to process.
 */
void system_input(BrRegistry *registry, BrEvent e);

/**
 * @brief System responsible for updating Entity position based on their
 * velocity.
 *
 * @param registry The central ECS data store.
 * @param delta_time The time elapsed since last frame (in seconds).
 */
void system_movement(BrRegistry *registry, double delta_time);

/**
 * @brief System responsible for controlling the player entity.
 *
 * @param registry The central ECS data store.
 */
void system_player_controller(BrRegistry *registry);

#endif // BR_ECS_H
