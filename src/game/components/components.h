#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "borka.h"
#include "collider.h"
#include "input_controlled.h"
#include "movement_config.h"
#include "position.h"
#include "renderable.h"
#include "velocity.h"

extern BrComponentTypeId COMPONENT_POSITION;
extern BrComponentTypeId COMPONENT_VELOCITY;
extern BrComponentTypeId COMPONENT_RENDERABLE;
extern BrComponentTypeId COMPONENT_MOVEMENT_CONFIG;
extern BrComponentTypeId COMPONENT_INPUT_CONTROLLED;
extern BrComponentTypeId COMPONENT_COLLIDER;

bool components_register(BrRegistry *registry);

#endif // COMPONENTS_H
