#ifndef ENTITIES_H
#define ENTITIES_H

#include "assets/assets.h"
#include "borka.h"

BrEntity player_create(BrRegistry *registry, const Assets *assets);
bool ground_create(BrRegistry *registry, const Assets *assets);

#endif // ENTITIES_H
