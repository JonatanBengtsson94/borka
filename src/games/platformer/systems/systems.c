#include "systems.h"
#include "components/components.h"

BrSystemId SYSTEM_RENDER = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_INPUT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PLAYER_ACTIONS = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_MOVEMENT = BR_INVALID_SYSTEM_ID;

bool systems_register(BrRegistry *registry) {
  BrComponentTypeId render_required[] = {COMPONENT_RENDERABLE,
                                         COMPONENT_POSITION};
  BrComponentTypeId movement_required[] = {COMPONENT_POSITION,
                                           COMPONENT_VELOCITY};
  BrComponentTypeId input_required[] = {COMPONENT_INPUT_CONTROLLED};
  BrComponentTypeId player_actions_required[] = {COMPONENT_INPUT_CONTROLLED,
                                                 COMPONENT_VELOCITY};

  SYSTEM_RENDER =
      br_system_register(registry, COMPONENT_RENDERABLE, render_required, 2);
  SYSTEM_MOVEMENT =
      br_system_register(registry, COMPONENT_VELOCITY, movement_required, 2);
  SYSTEM_INPUT = br_system_register(registry, COMPONENT_INPUT_CONTROLLED,
                                    input_required, 1);
  SYSTEM_PLAYER_ACTIONS = br_system_register(
      registry, COMPONENT_INPUT_CONTROLLED, player_actions_required, 2);

  BrSystemId ids[] = {SYSTEM_RENDER, SYSTEM_MOVEMENT, SYSTEM_INPUT,
                      SYSTEM_PLAYER_ACTIONS};

  size_t length = sizeof(ids) / sizeof(BrSystemId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_SYSTEM_ID) {
      BR_LOG_ERROR("Failed to register a system");
      return false;
    }
  }

  return true;
}
