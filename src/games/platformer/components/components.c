#include "components.h"

BrComponentTypeId COMPONENT_POSITION = BR_INVALID_COMPONENT_TYPE;
BrComponentTypeId COMPONENT_RENDERABLE = BR_INVALID_COMPONENT_TYPE;

bool components_register(BrRegistry *registry) {
  COMPONENT_POSITION = br_register_component(registry, sizeof(Position));
  COMPONENT_RENDERABLE = br_register_component(registry, sizeof(Renderable));

  BrComponentTypeId ids[] = {COMPONENT_POSITION, COMPONENT_RENDERABLE};

  size_t length = sizeof(ids) / sizeof(BrComponentTypeId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_COMPONENT_TYPE) {
      BR_LOG_ERROR("Failed to register a component type");
      return false;
    }
  }

  return true;
}
