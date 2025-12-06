#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "borka.h"

void system_input(BrRegistry *registry, BrEvent e);
void system_player_movement(BrRegistry *registry);
void system_render(BrRegistry *registry, BrRenderer *renderer);
void system_movement(BrRegistry *registry, double delta_time);
void system_collision(BrRegistry *registry);

extern BrSystemId SYSTEM_INPUT;
extern BrSystemId SYSTEM_PLAYER_MOVEMENT;
extern BrSystemId SYSTEM_RENDER;
extern BrSystemId SYSTEM_MOVEMENT;
extern BrSystemId SYSTEM_COLLISION;

bool systems_register(BrRegistry *registry);

#endif // SYSTEMS_H
