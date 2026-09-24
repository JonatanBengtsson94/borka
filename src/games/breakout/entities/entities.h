#include "borka.h"
#include "game.h"

void create_paddle(BrRegistry *registry, BrTexture *texture);
void create_ball(BrRegistry *registry, BrTexture *texture);
void create_walls(BrRegistry *registry);
void create_bricks(GameState *game);
void create_main_menu(BrRegistry *registry, BrFont *font);
void create_game_over(BrRegistry *registry, BrFont *font, char *title,
                      char *score_text, char *highscore_text);
