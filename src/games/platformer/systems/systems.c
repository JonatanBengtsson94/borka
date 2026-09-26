#include "systems.h"
#include "components/components.h"

BrSystemId SYSTEM_RENDER = BR_INVALID_SYSTEM_ID;

bool systems_register(BrRegistry *registry) {
  BrComponentTypeId render_required[] = {COMPONENT_RENDERABLE,
                                         COMPONENT_POSITION};

  SYSTEM_RENDER =
      br_register_system(registry, COMPONENT_RENDERABLE, render_required, 2);

  BrSystemId ids[] = {SYSTEM_RENDER};

  size_t length = sizeof(ids) / sizeof(BrSystemId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_SYSTEM_ID) {
      BR_LOG_ERROR("Failed to register a system");
      return false;
    }
  }

  return true;
}
