#ifndef GAME_H
#define GAME_H

#include "borka.h"
#include <stdbool.h>

typedef struct {
  BrApp *app;
  BrFont font;

  struct {
    BrTexture *paddle;
    BrTexture *ball;
    BrTexture *brick_blue;
    BrTexture *brick_red;
    BrTexture *brick_green;
  } textures;

  struct {
    BrSound *bounce_sound;
  } sfx;

  int enemies_alive;
  int level;
  bool is_paused;
  bool game_over;
} GameState;

bool game_init(GameState *game);
void game_update(GameState *game, double delta_time);
void game_handle_event(GameState *game, BrEvent event);
void game_shutdown(GameState *game);

#endif // GAME_H
