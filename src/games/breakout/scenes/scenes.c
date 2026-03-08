#include "scenes.h"
#include "entities/entities.h"

static void destroy_scene(BrRegistry *reg) {
  assert(reg);
  for (int i = 0; i < MAX_ENTITIES; i++) {
    br_entity_destroy(reg, i);
  }
}

void create_start_scene(GameState *game) {
  assert(game);
  destroy_scene(game->app->registry);
  BR_LOG_DEBUG("Creating start scene");

  create_main_menu(game->app->registry, &game->font);

  game->level = 0;
}

void create_level_01_scene(GameState *game) {
  destroy_scene(game->app->registry);
  BR_LOG_DEBUG("Creating level 1 scene");

  create_paddle(game->app->registry, game->textures.paddle);
  create_ball(game->app->registry, game->textures.ball);
  create_walls(game->app->registry);
  create_bricks(game);

  game->level = 1;
  game->game_over = false;
}
