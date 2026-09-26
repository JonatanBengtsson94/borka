#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "borka.h"

#include "position.h"
#include "renderable.h"

extern BrComponentTypeId COMPONENT_POSITION;
extern BrComponentTypeId COMPONENT_RENDERABLE;

bool components_register(BrRegistry *registry);

#endif // COMPONENTS_H
