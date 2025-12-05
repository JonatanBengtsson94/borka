#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "borka.h"

/**
 * @brief System responsible for updating all entities based on input events.
 *
 * @param registry The central ECS data store.
 * @param e The event to process.
 */
void system_input(BrRegistry *registry, BrEvent e);

/**
 * @brief System responsible for controlling the player entity.
 *
 * @param registry The central ECS data store.
 */
void system_player_movement(BrRegistry *registry);

/**
 * @brief System responsible for rendering all visible Entities to the screen.
 *
 * @param registry The central ECS data store.
 * @param app The main application struct.
 */
void system_render(BrRegistry *registry, BrRenderer *renderer);

/**
 * @brief System responsible for updating Entity position based on their
 * velocity.
 *
 * @param registry The central ECS data store.
 * @param delta_time The time elapsed since last frame (in seconds).
 */
void system_physics(BrRegistry *registry, double delta_time);

extern BrSystemId SYSTEM_INPUT;
extern BrSystemId SYSTEM_PLAYER_MOVEMENT;
extern BrSystemId SYSTEM_RENDER;
extern BrSystemId SYSTEM_PHYSICS;

bool systems_register(BrRegistry *registry);

#endif // SYSTEMS_H
