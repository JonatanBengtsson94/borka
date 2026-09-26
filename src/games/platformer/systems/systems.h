#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "borka.h"
#include "game.h"

void system_render(BrRegistry *registry, BrRenderer *renderer);

extern BrSystemId SYSTEM_RENDER;

bool systems_register(BrRegistry *registry);

#endif // SYSTEMS_H
