#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "borka.h"

#include "animator.h"
#include "brick.h"
#include "collider.h"
#include "collision.h"
#include "input_controlled.h"
#include "movement_config.h"
#include "position.h"
#include "renderable.h"
#include "velocity.h"

extern BrComponentTypeId COMPONENT_ANIMATOR;
extern BrComponentTypeId COMPONENT_POSITION;
extern BrComponentTypeId COMPONENT_VELOCITY;
extern BrComponentTypeId COMPONENT_RENDERABLE;
extern BrComponentTypeId COMPONENT_MOVEMENT_CONFIG;
extern BrComponentTypeId COMPONENT_INPUT_CONTROLLED;
extern BrComponentTypeId COMPONENT_COLLIDER;
extern BrComponentTypeId COMPONENT_COLLISION;
extern BrComponentTypeId COMPONENT_BRICK;

bool components_register(BrRegistry *registry);

#endif // COMPONENTS_H
