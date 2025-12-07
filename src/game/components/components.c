#include "components.h"

BrComponentTypeId COMPONENT_POSITION = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_VELOCITY = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_RENDERABLE = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_MOVEMENT_CONFIG = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_INPUT_CONTROLLED = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_COLLIDER = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_COLLISION = BR_INVALID_COMPONENT_TYPE;

bool components_register(BrRegistry *registry) {
  COMPONENT_POSITION = br_register_component(registry, sizeof(Position));
  COMPONENT_VELOCITY = br_register_component(registry, sizeof(Velocity));
  COMPONENT_RENDERABLE = br_register_component(registry, sizeof(Renderable));
  COMPONENT_MOVEMENT_CONFIG =
      br_register_component(registry, sizeof(MovementConfig));
  COMPONENT_INPUT_CONTROLLED =
      br_register_component(registry, sizeof(InputControlled));
  COMPONENT_COLLIDER = br_register_component(registry, sizeof(Collider));
  COMPONENT_COLLISION = br_register_component(registry, sizeof(Collision));

  BrComponentTypeId ids[] = {
      COMPONENT_POSITION,        COMPONENT_VELOCITY,
      COMPONENT_RENDERABLE,      COMPONENT_MOVEMENT_CONFIG,
      COMPONENT_MOVEMENT_CONFIG, COMPONENT_COLLIDER,
      COMPONENT_COLLISION};

  size_t length = sizeof(ids) / sizeof(BrComponentTypeId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_COMPONENT_TYPE) {
      BR_LOG_ERROR("Failed to register a component type");
      return false;
    }
  }

  return true;
}
