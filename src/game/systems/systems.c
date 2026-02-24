#include "systems.h"
#include "components/components.h"

BrSystemId SYSTEM_INPUT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PLAYER_MOVEMENT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_RENDER = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_MOVEMENT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_COLLISION_DETECTION = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_COLLISION_HANDLING = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_ANIMATION = BR_INVALID_SYSTEM_ID;

bool systems_register(BrRegistry *registry) {
  BrComponentTypeId input_required[] = {COMPONENT_INPUT_CONTROLLED};
  BrComponentTypeId render_required[] = {COMPONENT_RENDERABLE,
                                         COMPONENT_POSITION};
  BrComponentTypeId physics_required[] = {COMPONENT_VELOCITY,
                                          COMPONENT_POSITION};
  BrComponentTypeId player_movement_required[] = {
      COMPONENT_INPUT_CONTROLLED, COMPONENT_MOVEMENT_CONFIG, COMPONENT_VELOCITY,
      COMPONENT_COLLIDER};
  BrComponentTypeId collision_detection_required[] = {COMPONENT_POSITION,
                                                      COMPONENT_COLLIDER};
  BrComponentTypeId collision_handling_required[] = {COMPONENT_COLLISION,
                                                     COMPONENT_COLLIDER};
  BrComponentTypeId animation_required[] = {COMPONENT_ANIMATOR,
                                            COMPONENT_RENDERABLE};

  SYSTEM_INPUT = br_register_system(registry, COMPONENT_INPUT_CONTROLLED,
                                    input_required, 1);
  SYSTEM_RENDER =
      br_register_system(registry, COMPONENT_RENDERABLE, render_required, 2);
  SYSTEM_MOVEMENT =
      br_register_system(registry, COMPONENT_VELOCITY, physics_required, 2);
  SYSTEM_PLAYER_MOVEMENT = br_register_system(
      registry, COMPONENT_INPUT_CONTROLLED, player_movement_required, 4);
  SYSTEM_COLLISION_DETECTION = br_register_system(
      registry, COMPONENT_COLLIDER, collision_detection_required, 2);
  SYSTEM_COLLISION_HANDLING = br_register_system(
      registry, COMPONENT_COLLISION, collision_handling_required, 2);
  SYSTEM_ANIMATION =
      br_register_system(registry, COMPONENT_ANIMATOR, animation_required, 2);

  BrSystemId ids[] = {SYSTEM_INPUT,
                      SYSTEM_RENDER,
                      SYSTEM_MOVEMENT,
                      SYSTEM_PLAYER_MOVEMENT,
                      SYSTEM_COLLISION_DETECTION,
                      SYSTEM_COLLISION_HANDLING,
                      SYSTEM_ANIMATION};

  size_t length = sizeof(ids) / sizeof(BrSystemId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_SYSTEM_ID) {
      BR_LOG_ERROR("Failed to register a system");
      return false;
    }
  }

  return true;
}
