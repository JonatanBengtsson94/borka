#include "systems.h"
#include "components/components.h"

BrSystemId SYSTEM_INPUT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PLAYER_MOVEMENT = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_RENDER = BR_INVALID_SYSTEM_ID;
BrSystemId SYSTEM_PHYSICS = BR_INVALID_SYSTEM_ID;

bool systems_register(BrRegistry *registry) {
  BrComponentTypeId input_required[] = {COMPONENT_INPUT_CONTROLLED};
  BrComponentTypeId render_required[] = {COMPONENT_SPRITE, COMPONENT_POSITION};
  BrComponentTypeId physics_required[] = {COMPONENT_VELOCITY,
                                          COMPONENT_POSITION};
  BrComponentTypeId player_movement_required[] = {COMPONENT_INPUT_CONTROLLED,
                                                  COMPONENT_MOVEMENT_CONFIG,
                                                  COMPONENT_VELOCITY};

  SYSTEM_INPUT = br_register_system(registry, input_required, 1);
  SYSTEM_RENDER = br_register_system(registry, render_required, 2);
  SYSTEM_PHYSICS = br_register_system(registry, physics_required, 2);
  SYSTEM_PLAYER_MOVEMENT =
      br_register_system(registry, player_movement_required, 3);

  BrSystemId ids[] = {SYSTEM_INPUT, SYSTEM_RENDER, SYSTEM_PHYSICS,
                      SYSTEM_PLAYER_MOVEMENT};
  size_t length = sizeof(ids) / sizeof(BrSystemId);
  for (size_t i = 0; i < length; i++) {
    if (ids[i] == BR_INVALID_SYSTEM_ID) {
      BR_LOG_ERROR("Failed to register a system");
      return false;
    }
  }

  return true;
}

void system_input(BrRegistry *registry, BrEvent e) {
  if (!registry) {
    BR_LOG_ERROR("Could not process input, registry is NULL");
    return;
  }

  bool pressed;
  if (e.type == BR_EVENT_KEY_PRESSED) {
    BR_LOG_TRACE("Key %d pressed", e.data.keycode);
    pressed = true;
  } else if (e.type == BR_EVENT_KEY_RELEASED) {
    BR_LOG_TRACE("Key %d released", e.data.keycode);
    pressed = false;
  }

  BrComponentArray *input_controlled_array =
      registry->component_arrays[COMPONENT_INPUT_CONTROLLED];

  if (!input_controlled_array) {
    BR_LOG_DEBUG("Nothing to control. input controlled array uninitialized");
    return;
  }

  InputControlled *input_controls = input_controlled_array->components.data;
  size_t count = input_controlled_array->entity_ids.length;

  for (size_t i = 0; i < count; i++) {
    InputControlled *input_control = &input_controls[i];

    switch (e.data.keycode) {
    case 30:
      input_control->left_pressed = pressed;
      break;

    case 32:
      input_control->right_pressed = pressed;
      break;
    }
  }
}

void system_render(BrRegistry *registry, BrRenderer *renderer) {
  const BrSignature system_signature =
      registry->system_signatures[SYSTEM_RENDER];

  if (!registry) {
    BR_LOG_ERROR("Could not run rendering system, registry is NULL");
    return;
  }

  if (!renderer) {
    BR_LOG_ERROR("Could not run rendering system, renderer is NULL");
    return;
  }
  br_renderer_clear(renderer, 0xFF000000);

  BrComponentArray *sprite_array = registry->component_arrays[COMPONENT_SPRITE];
  BrComponentArray *position_array =
      registry->component_arrays[COMPONENT_POSITION];
  if (!sprite_array || !position_array) {
    BR_LOG_DEBUG("Nothing to render. sprite or position array uninitialized");
    br_renderer_present(renderer);
    return;
  }

  BrEntity *ids = (BrEntity *)sprite_array->entity_ids.data;
  int *position_sparse_map = position_array->entity_to_index;

  Sprite *sprites = (Sprite *)sprite_array->components.data;
  Position *positions = (Position *)position_array->components.data;
  size_t count = sprite_array->components.length;

  for (size_t i = 0; i < count; i++) {
    Sprite *sprite = &sprites[i];
    BrEntity entity = ids[i];
    if ((registry->entity_signatures[entity] & system_signature))
      continue;
    int position_index = position_sparse_map[entity];
    Position *position = &positions[position_index];
    br_renderer_draw_texture(renderer, position->x, position->y,
                             sprite->texture);
  }

  br_renderer_present(renderer);
}

void system_physics(BrRegistry *registry, double delta_time) {
  const BrSignature system_signature =
      registry->system_signatures[SYSTEM_PHYSICS];

  if (!registry) {
    BR_LOG_ERROR("Could not run physics system, registry is NULL");
    return;
  }

  BrComponentArray *velocities_array =
      registry->component_arrays[COMPONENT_VELOCITY];
  BrComponentArray *positions_array =
      registry->component_arrays[COMPONENT_POSITION];
  if (!velocities_array || !positions_array) {
    BR_LOG_DEBUG("Nothing to perform physics on. velocity or position array "
                 "uninitialized");
    return;
  }

  BrEntity *ids = (BrEntity *)velocities_array->entity_ids.data;
  int *positions_sparse_map = positions_array->entity_to_index;

  Velocity *velocities = (Velocity *)velocities_array->components.data;
  Position *positions = (Position *)positions_array->components.data;
  size_t count = velocities_array->entity_ids.length;

  for (size_t i = 0; i < count; i++) {
    Velocity *velocity = &velocities[i];
    BrEntity entity = ids[i];
    if ((registry->entity_signatures[entity] & system_signature))
      continue;
    int position_index = positions_sparse_map[entity];
    Position *position = &positions[position_index];
    position->x += velocity->vx * delta_time;
    position->y += velocity->vy * delta_time;
  }
}

void system_player_movement(BrRegistry *registry) {
  const BrSignature system_signature =
      registry->system_signatures[SYSTEM_PLAYER_MOVEMENT];

  if (!registry) {
    BR_LOG_ERROR("Could not apply player movement, registry is NULL");
  }

  BrComponentArray *input_controlled_array =
      registry->component_arrays[COMPONENT_INPUT_CONTROLLED];
  BrComponentArray *movement_config_array =
      registry->component_arrays[COMPONENT_MOVEMENT_CONFIG];
  BrComponentArray *velocity_array =
      registry->component_arrays[COMPONENT_VELOCITY];

  if (!input_controlled_array || !movement_config_array || !velocity_array) {
    BR_LOG_DEBUG("No player movement. Input control, movement config or "
                 "velocity array is NULL");
    return;
  }

  InputControlled *input_controls = input_controlled_array->components.data;
  MovementConfig *movement_configs = movement_config_array->components.data;
  Velocity *velocities = velocity_array->components.data;

  size_t count = input_controlled_array->entity_ids.length;
  BrEntity *ids = (BrEntity *)input_controlled_array->entity_ids.data;
  int *movement_config_sparse_map = movement_config_array->entity_to_index;
  int *velocities_sparse_map = velocity_array->entity_to_index;

  for (size_t i = 0; i < count; i++) {
    InputControlled *input_control = &input_controls[i];
    BrEntity entity = ids[i];
    if ((registry->entity_signatures[entity] & system_signature))
      continue;
    int movement_config_index = movement_config_sparse_map[entity];
    int velocity_index = velocities_sparse_map[entity];
    MovementConfig *movement_config = &movement_configs[movement_config_index];
    Velocity *velocity = &velocities[velocity_index];

    int horizontal = 0;
    float speed = movement_config->move_speed;

    if (input_control->left_pressed && !input_control->right_pressed)
      horizontal = -1;
    else if (input_control->right_pressed && !input_control->left_pressed)
      horizontal = 1;
    else
      horizontal = 0;

    velocity->vx = horizontal * speed;
  }
}
