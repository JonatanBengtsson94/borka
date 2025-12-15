#include "game.h"
#include "components/components.h"
#include "entities/entities.h"
#include "scenes/scenes.h"
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

  BrTexture *font_atlas = br_texture_create("assets/fonts/font_atlas.png");
  if (!font_atlas) {
    BR_LOG_ERROR("Failed to load font atlas");
    goto error;
  }
  BrFont font = {
      .glyph_size = {8, 8}, .font_atlas = font_atlas, .spacing = {2, 2}};
  game->font = font;

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

  game->textures.brick_green =
      br_texture_create("assets/textures/brick_green.png");
  if (!game->textures.brick_green) {
    BR_LOG_ERROR("Failed to load ball texture");
    goto error;
  }

  game->textures.brick_blue =
      br_texture_create("assets/textures/brick_blue.png");
  if (!game->textures.brick_blue) {
    BR_LOG_ERROR("Failed to load ball texture");
    goto error;
  }

  game->textures.brick_red = br_texture_create("assets/textures/brick_red.png");
  if (!game->textures.brick_red) {
    BR_LOG_ERROR("Failed to load ball texture");
    goto error;
  }

  game->is_paused = false;
  game->enemies_alive = 0;

  create_start_scene(game);

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
  if (game->textures.brick_green)
    br_texture_destroy(game->textures.brick_green);
  if (game->textures.brick_blue)
    br_texture_destroy(game->textures.brick_blue);
  if (game->textures.brick_red)
    br_texture_destroy(game->textures.brick_red);
  if (game->app)
    br_app_destroy(game->app);
}

void game_handle_event(GameState *game, BrEvent event) {
  if (event.type == BR_EVENT_KEY_PRESSED ||
      event.type == BR_EVENT_KEY_RELEASED) {
    if (game->level == 0) {
      create_level_01_scene(game);
    }
    system_input(game, event);
  }
}

void game_update(GameState *game, double delta_time) {
  if (game->is_paused)
    return;
  if (game->game_over && game->level != 0) {
    create_start_scene(game);
    return;
  }
  system_player_movement(game->app->registry);
  system_movement(game->app->registry, delta_time);
  system_collision_detection(game->app->registry);
  system_collision_handling(game);
  system_render(game->app->registry, game->app->renderer);
}
