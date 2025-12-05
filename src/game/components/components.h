#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "borka.h"

/**
 * @brief Holds the position of an Entity in the game world.
 */
typedef struct {
  float x; /**< X-coordinate */
  float y; /**< Y-coordinate */
} Position;

/**
 * @brief Holds the velocity (change in position per second) of an Entity.
 */
typedef struct {
  float vx; /**< Velocity along the X-axis. */
  float vy; /**< Velocity along the Y-axis. */
} Velocity;

/**
 * @brief Holds the visual asset data for rendering an Entity.
 */
typedef struct {
  BrTexture *texture; /**< Pointer to the texture instance. */
} Sprite;

/**
 * @brief Stores the configuration for movement.
 */
typedef struct {
  float move_speed; /**< The maximum movement speed (pixels per second). */
} MovementConfig;

/**
 * @brief Stores input data.
 */
typedef struct {
  bool left_pressed;  /** True if left key is held down. */
  bool right_pressed; /** True if right key is held down. */
} InputControlled;

extern BrComponentTypeId COMPONENT_POSITION;
extern BrComponentTypeId COMPONENT_VELOCITY;
extern BrComponentTypeId COMPONENT_SPRITE;
extern BrComponentTypeId COMPONENT_MOVEMENT_CONFIG;
extern BrComponentTypeId COMPONENT_INPUT_CONTROLLED;

bool components_register(BrRegistry *registry);

#endif // COMPONENTS_H
