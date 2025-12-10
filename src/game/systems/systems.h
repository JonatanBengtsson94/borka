#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "borka.h"
#include "game.h"

void system_input(GameState *game, BrEvent e);
void system_player_movement(BrRegistry *registry);
void system_render(BrRegistry *registry, BrRenderer *renderer);
void system_movement(BrRegistry *registry, double delta_time);
void system_collision_detection(BrRegistry *registry);
void system_collision_handling(GameState *game);

extern BrSystemId SYSTEM_INPUT;
extern BrSystemId SYSTEM_PLAYER_MOVEMENT;
extern BrSystemId SYSTEM_RENDER;
extern BrSystemId SYSTEM_MOVEMENT;
extern BrSystemId SYSTEM_COLLISION_DETECTION;
extern BrSystemId SYSTEM_COLLISION_HANDLING;

bool systems_register(BrRegistry *registry);

#endif // SYSTEMS_H
