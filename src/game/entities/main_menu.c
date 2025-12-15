#include "components/components.h"
#include "constants.h"
#include "entities.h"

void create_main_menu(BrRegistry *registry, BrFont *font) {
  BrEntity txt = br_entity_create(registry);
  Position pos = {50, GAME_HEIGHT / 2};
  Renderable ren = {.type = RENDERABLE_TEXT,
                    .text = {.font = font, .text = "PRESS ANY KEY TO START"}};
  br_component_add(registry, txt, COMPONENT_POSITION, &pos);
  br_component_add(registry, txt, COMPONENT_RENDERABLE, &ren);
}
