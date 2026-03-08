#include "borka.h"
#include "game.h"

void create_paddle(BrRegistry *registry, BrTexture *texture);
void create_ball(BrRegistry *registry, BrTexture *texture);
void create_walls(BrRegistry *registry);
void create_bricks(GameState *game);
void create_main_menu(BrRegistry *registry, BrFont *font);
