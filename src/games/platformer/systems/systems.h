#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "borka.h"
#include "game.h"

void system_render(BrRegistry *registry, BrRenderer *renderer);
void system_input(BrRegistry *registry, BrEvent event);
void system_movement(BrRegistry *registry, double delta_time);
void system_player_actions(BrRegistry *registry);

extern BrSystemId SYSTEM_RENDER;
extern BrSystemId SYSTEM_PLAYER_ACTIONS;
extern BrSystemId SYSTEM_INPUT;
extern BrSystemId SYSTEM_MOVEMENT;

bool systems_register(BrRegistry *registry);

#endif // SYSTEMS_H
