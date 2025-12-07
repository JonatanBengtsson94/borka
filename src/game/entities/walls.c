#include "components/components.h"
#include "entities.h"

void create_walls(BrRegistry *registry, int width, int height) {
  // Left Wall
  BrEntity left_wall = br_entity_create(registry);
  Position left_wall_pos = {0, 0};
  Collider left_wall_col = {
      .size = {1, height}, .layer = LAYER_WALL, .mask = LAYER_BALL};
  br_component_add(registry, left_wall, COMPONENT_POSITION, &left_wall_pos);
  br_component_add(registry, left_wall, COMPONENT_COLLIDER, &left_wall_col);

  // Right Wall
  BrEntity right_wall = br_entity_create(registry);
  Position right_wall_pos = {200, 0};
  Collider right_wall_col = {
      .size = {1, height}, .layer = LAYER_WALL, .mask = LAYER_BALL};
  br_component_add(registry, right_wall, COMPONENT_POSITION, &right_wall_pos);
  br_component_add(registry, right_wall, COMPONENT_COLLIDER, &right_wall_col);

  // Top Wall
  BrEntity top_wall = br_entity_create(registry);
  Position top_wall_pos = {0, -1};
  Collider top_wall_col = {
      .size = {width, 1}, .layer = LAYER_WALL, .mask = LAYER_BALL};
  br_component_add(registry, top_wall, COMPONENT_POSITION, &top_wall_pos);
  br_component_add(registry, top_wall, COMPONENT_COLLIDER, &top_wall_col);
}
