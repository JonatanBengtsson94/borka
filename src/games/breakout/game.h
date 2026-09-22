#ifndef GAME_H
#define GAME_H

#include "borka.h"
#include <stdbool.h>

// Both audio assets are normalised to the same peak, so these only set the
// balance: the music sits well back, leaving room for bounces on top of it.
#define MUSIC_VOLUME 0.3f
#define BOUNCE_VOLUME 0.7f

typedef struct {
  BrApp *app;
  BrFont font;

  struct {
    BrTexture *paddle;
    BrTexture *ball;
    BrTexture *background;
    BrTextureRegion brick_blue;
    BrTextureRegion brick_red;
    BrTextureRegion brick_green;
  } textures;

  struct {
    BrTextureRegion brick_blue_break[3];
    BrTextureRegion brick_red_break[3];
    BrTextureRegion brick_green_break[3];
  } animations;

  struct {
    BrSound *bounce_sound;
  } sfx;

  struct {
    BrSound *menu;
    BrSound *gameplay;
  } music;

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
