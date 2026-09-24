#ifndef GAME_H
#define GAME_H

#include "borka.h"
#include "camera.h"
#include "constants.h"
#include "scenes/scene.h"
#include <stdbool.h>

// Both audio assets are normalised to the same peak, so these only set the
// balance: the music sits well back, leaving room for hits on top of it.
#define MUSIC_VOLUME 0.3f
#define SFX_VOLUME 0.7f

typedef struct {
  BrApp *app;
  BrFont font;

  struct {
    BrTexture *paddle;
    BrTexture *ball;
    BrTexture *ball_squash;
    BrTexture *background;
    BrTextureRegion brick_blue;
    BrTextureRegion brick_red;
    BrTextureRegion brick_green;
    BrTextureRegion trail[TRAIL_LENGTH]; // Largest, nearest the ball, first.
  } textures;

  struct {
    BrTextureRegion brick_blue_break[3];
    BrTextureRegion brick_red_break[3];
    BrTextureRegion brick_green_break[3];
    // Squash after a bounce, ending on the round ball. The offsets keep each
    // frame centred on the ball's 8x8 collider.
    BrTextureRegion ball_squash_vertical[3];
    BrVec2 ball_squash_vertical_offsets[3];
    BrTextureRegion ball_squash_horizontal[3];
    BrVec2 ball_squash_horizontal_offsets[3];
  } animations;

  struct {
    BrSound *paddle_hit;
    BrSound *wall_hit;
    BrSound *brick_hit;
  } sfx;

  struct {
    BrSound *menu;
    BrSound *gameplay;
  } music;

  Scene scene;
  Camera camera;

  int enemies_alive;
  int score;
  int highscore; // Best score this session. Never reset, so not saved.
  bool won;
  // Backing storage for the game over scene's score lines. Text renderables
  // only borrow their string, so it has to outlive the scene.
  char score_text[32];
  char highscore_text[32];
  bool is_paused;
  bool game_over;
} GameState;

bool game_init(GameState *game);
void game_update(GameState *game, double delta_time);
void game_handle_event(GameState *game, BrEvent event);
void game_shutdown(GameState *game);

#endif // GAME_H
