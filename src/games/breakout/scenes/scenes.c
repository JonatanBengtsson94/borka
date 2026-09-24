#include "scenes.h"
#include "entities/entities.h"
#include <stdio.h>

static void destroy_scene(BrRegistry *reg) {
  assert(reg);
  for (int i = 0; i < MAX_ENTITIES; i++) {
    if (br_entity_is_alive(reg, i))
      br_entity_destroy(reg, i);
  }
}

// Only the outgoing scene's track can be looping, so stopping it is enough to
// keep tracks from stacking. A track shared by both scenes keeps playing
// instead of restarting.
static void switch_music(BrSound *current, BrSound *next) {
  if (current == next)
    return;
  if (current)
    br_stop_sound(current);
  if (next)
    br_play_sound_looping(next, MUSIC_VOLUME);
}

void scene_load(GameState *game, SceneId id) {
  assert(game);
  BrRegistry *registry = game->app->registry;
  destroy_scene(registry);

  Scene next = {.id = id};

  switch (id) {
  case SCENE_NONE:
    break;

  case SCENE_START:
    BR_LOG_DEBUG("Loading start scene");
    create_main_menu(registry, &game->font);
    next.background = game->textures.background;
    next.music = game->music.menu;
    break;

  case SCENE_LEVEL_01:
    BR_LOG_DEBUG("Loading level 1 scene");
    game->enemies_alive = 0;
    game->score = 0;
    game->won = false;
    create_paddle(registry, game->textures.paddle);
    create_ball(registry, game->textures.ball);
    create_walls(registry);
    create_bricks(game);
    next.background = game->textures.background;
    next.music = game->music.gameplay;
    game->game_over = false;
    break;

  case SCENE_GAME_OVER:
    BR_LOG_DEBUG("Loading game over scene");
    if (game->score > game->highscore)
      game->highscore = game->score;
    snprintf(game->score_text, sizeof(game->score_text), "SCORE %d",
             game->score);
    snprintf(game->highscore_text, sizeof(game->highscore_text), "BEST %d",
             game->highscore);
    create_game_over(registry, &game->font,
                     game->won ? "YOU WIN" : "GAME OVER", game->score_text,
                     game->highscore_text);
    next.background = game->textures.background;
    next.music = game->music.menu;
    break;
  }

  switch_music(game->scene.music, next.music);
  game->scene = next;
}
