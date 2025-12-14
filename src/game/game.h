#ifndef GAME_H
#define GAME_H

#include "borka.h"
#include <stdbool.h>

typedef struct {
  BrApp *app;

  struct {
    BrTexture *paddle;
    BrTexture *ball;
    BrTexture *brick;
  } textures;

  int enemies_alive;
  bool is_paused;
  bool game_over;
  bool level_loaded;
} GameState;

bool game_init(GameState *game);
void game_start(GameState *game);
void game_stop(GameState *game);
void game_update(GameState *game, double delta_time);
void game_handle_event(GameState *game, BrEvent event);
void game_shutdown(GameState *game);

#endif // GAME_H
