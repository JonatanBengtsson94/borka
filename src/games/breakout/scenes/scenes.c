#include "scenes.h"
#include "entities/entities.h"

static void destroy_scene(BrRegistry *reg) {
  assert(reg);
  for (int i = 0; i < MAX_ENTITIES; i++) {
    if (br_entity_is_alive(reg, i))
      br_entity_destroy(reg, i);
  }
}

// Only one track should be looping at a time. Both are stopped first so
// that re-entering a scene cannot stack a second copy of its own track on
// top of the first. Stopping a track that is not playing is a no-op.
static void play_music(BrSound *track, BrSound *other) {
  br_stop_sound(other);
  br_stop_sound(track);
  br_play_sound_looping(track, MUSIC_VOLUME);
}

void create_start_scene(GameState *game) {
  assert(game);
  destroy_scene(game->app->registry);
  BR_LOG_DEBUG("Creating start scene");

  create_main_menu(game->app->registry, &game->font);
  play_music(game->music.menu, game->music.gameplay);

  game->level = 0;
}

void create_level_01_scene(GameState *game) {
  destroy_scene(game->app->registry);
  BR_LOG_DEBUG("Creating level 1 scene");

  create_paddle(game->app->registry, game->textures.paddle);
  create_ball(game->app->registry, game->textures.ball);
  create_walls(game->app->registry);
  create_bricks(game);
  play_music(game->music.gameplay, game->music.menu);

  game->level = 1;
  game->game_over = false;
}
