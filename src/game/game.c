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
  game->game_over = false;

  create_paddle(game->app->registry, game->textures.paddle);
  create_ball(game->app->registry, game->textures.ball);
  create_walls(game->app->registry);
  create_bricks(game);

  return true;

error:
  game_shutdown(game);
  return false;
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
  if (event.type == BR_EVENT_KEY_PRESSED || event.type == BR_EVENT_KEY_RELEASED)
    system_input(game, event);
}

void game_update(GameState *game, double delta_time) {
  system_player_movement(game->app->registry);
  system_movement(game->app->registry, delta_time);
  system_collision_detection(game->app->registry);
  system_collision_handling(game);
  system_render(game->app->registry, game->app->renderer);
}
