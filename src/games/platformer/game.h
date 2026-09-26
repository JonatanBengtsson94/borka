#ifndef GAME_H
#define GAME_H

#include "assets/assets.h"
#include "borka.h"

typedef struct {
  BrApp *app; // Borrowed, main owns it.
  Assets assets;
} Game;

void game_update(Game *game);
// Expects a zeroed Game with app set.
bool game_init(Game *game);
void game_shutdown(Game *game);

#endif // GAME_H
