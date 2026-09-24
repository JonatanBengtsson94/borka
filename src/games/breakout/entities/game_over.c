#include "components/components.h"
#include "constants.h"
#include "entities.h"

static void create_centered_text(BrRegistry *registry, BrFont *font,
                                 char *text, float y) {
  BrEntity txt = br_entity_create(registry);
  Position pos = {(GAME_WIDTH - br_font_text_width(font, text)) / 2, y};
  Renderable ren = {.type = RENDERABLE_TEXT,
                    .text = {.font = font, .text = text}};
  br_component_add(registry, txt, COMPONENT_POSITION, &pos);
  br_component_add(registry, txt, COMPONENT_RENDERABLE, &ren);
}

void create_game_over(BrRegistry *registry, BrFont *font, char *title,
                      char *score_text) {
  create_centered_text(registry, font, title, 60);
  create_centered_text(registry, font, score_text, 80);
  create_centered_text(registry, font, "PRESS ANY KEY TO PLAY AGAIN", 110);
}
