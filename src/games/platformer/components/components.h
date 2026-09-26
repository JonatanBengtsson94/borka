#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "borka.h"

#include "input_controlled.h"
#include "position.h"
#include "renderable.h"
#include "velocity.h"

extern BrComponentTypeId COMPONENT_POSITION;
extern BrComponentTypeId COMPONENT_RENDERABLE;
extern BrComponentTypeId COMPONENT_VELOCITY;
extern BrComponentTypeId COMPONENT_INPUT_CONTROLLED;

bool components_register(BrRegistry *registry);

#endif // COMPONENTS_H
