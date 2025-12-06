#include "systems.h"
#include "components/components.h"

BrSystemId SYSTEM_INPUT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PLAYER_MOVEMENT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_RENDER = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_MOVEMENT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_COLLISION = BR_INVALID_SYSTEM_ID;

bool systems_register(BrRegistry *registry) {
  BrComponentTypeId input_required[] = {COMPONENT_INPUT_CONTROLLED};
  BrComponentTypeId render_required[] = {COMPONENT_RENDERABLE,
                                         COMPONENT_POSITION};
  BrComponentTypeId physics_required[] = {COMPONENT_VELOCITY,
                                          COMPONENT_POSITION};
  BrComponentTypeId player_movement_required[] = {COMPONENT_INPUT_CONTROLLED,
                                                  COMPONENT_MOVEMENT_CONFIG,
                                                  COMPONENT_VELOCITY};
  BrComponentTypeId collision_required[] = {COMPONENT_POSITION,
                                            COMPONENT_COLLIDER};

  SYSTEM_INPUT = br_register_system(registry, COMPONENT_INPUT_CONTROLLED,
                                    input_required, 1);
  SYSTEM_RENDER =
      br_register_system(registry, COMPONENT_RENDERABLE, render_required, 2);
  SYSTEM_MOVEMENT =
      br_register_system(registry, COMPONENT_VELOCITY, physics_required, 2);
  SYSTEM_PLAYER_MOVEMENT = br_register_system(
      registry, COMPONENT_INPUT_CONTROLLED, player_movement_required, 3);
  SYSTEM_COLLISION =
      br_register_system(registry, COMPONENT_COLLIDER, collision_required, 2);

  BrSystemId ids[] = {SYSTEM_INPUT, SYSTEM_RENDER, SYSTEM_MOVEMENT,
                      SYSTEM_PLAYER_MOVEMENT, SYSTEM_COLLISION};
  size_t length = sizeof(ids) / sizeof(BrSystemId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_SYSTEM_ID) {
      BR_LOG_ERROR("Failed to register a system");
      return false;
    }
  }

  return true;
}
