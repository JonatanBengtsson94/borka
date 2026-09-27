#include "components.h"

BrComponentTypeId COMPONENT_POSITION = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_RENDERABLE = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_VELOCITY = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_INPUT_CONTROLLED = BR_INVALID_COMPONENT_TYPE;

bool components_register(BrRegistry *registry) {
  COMPONENT_POSITION = br_component_register(registry, sizeof(Position));
  COMPONENT_RENDERABLE = br_component_register(registry, sizeof(Renderable));
  COMPONENT_VELOCITY = br_component_register(registry, sizeof(Velocity));
  COMPONENT_INPUT_CONTROLLED =
      br_component_register(registry, sizeof(InputControlled));

  BrComponentTypeId ids[] = {COMPONENT_POSITION, COMPONENT_RENDERABLE,
                             COMPONENT_VELOCITY, COMPONENT_INPUT_CONTROLLED};

  size_t length = sizeof(ids) / sizeof(BrComponentTypeId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_COMPONENT_TYPE) {
      BR_LOG_ERROR("Failed to register a component type");
      return false;
    }
  }

  return true;
}
