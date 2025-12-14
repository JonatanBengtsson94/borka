#include "game.h"
#include "components/components.h"
#include "entities/entities.h"
#include "systems/systems.h"

bool game_init(GameState *game) {
  if (!components_register(game->app->registry)) {
    BR_LOG_ERROR("Failed to register components");
    goto error;
  }

  if (!systems_register(game->app->registry)) {
    BR_LOG_ERROR("Failed to register systems");
    goto error;
  }

  game->textures.paddle = br_texture_create("assets/textures/paddle.png");
  if (!game->textures.paddle) {
    BR_LOG_ERROR("Failed to load paddle texture");
    goto error;
  }

  game->textures.ball = br_texture_create("assets/textures/ball.png");
  if (!game->textures.ball) {
    BR_LOG_ERROR("Failed to load ball texture");
    goto error;
  }

  game->textures.brick = br_texture_create("assets/textures/brick.png");
  if (!game->textures.brick) {
    BR_LOG_ERROR("Failed to load ball texture");
    goto error;
  }

  game->is_paused = false;
  game->enemies_alive = 0;
  game->game_over = true;

  return true;

error:
  game_shutdown(game);
  return false;
}

void game_start(GameState *game) {
  BR_LOG_TRACE("Game started");
  create_paddle(game->app->registry, game->textures.paddle);
  create_ball(game->app->registry, game->textures.ball);
  create_walls(game->app->registry);
  create_bricks(game);
  game->game_over = false;
  game->level_loaded = true;
}

void game_stop(GameState *game) {
  BR_LOG_TRACE("Game stopped");
  for (int i = 0; i < MAX_ENTITIES; i++) {
    br_entity_destroy(game->app->registry, i);
  }
  game->level_loaded = false;
}

void game_shutdown(GameState *game) {
  if (game->textures.paddle)
    br_texture_destroy(game->textures.paddle);
  if (game->textures.ball)
    br_texture_destroy(game->textures.ball);
  if (game->textures.brick)
    br_texture_destroy(game->textures.brick);
  if (game->app)
    br_app_destroy(game->app);
}

void game_handle_event(GameState *game, BrEvent event) {
  if (event.type == BR_EVENT_KEY_PRESSED ||
      event.type == BR_EVENT_KEY_RELEASED) {
    if (game->game_over) {
      game_start(game);
    } else {
      system_input(game, event);
    }
  }
}

void game_update(GameState *game, double delta_time) {
  if (game->is_paused)
    return;
  if (game->game_over) {
    if (game->level_loaded)
      game_stop(game);
    // TODO: Render menu
    system_render(game->app->registry, game->app->renderer);
    BR_LOG_TRACE("Game is stopped, press any key to start");
    return;
  }
  system_player_movement(game->app->registry);
  system_movement(game->app->registry, delta_time);
  system_collision_detection(game->app->registry);
  system_collision_handling(game);
  system_render(game->app->registry, game->app->renderer);
}
