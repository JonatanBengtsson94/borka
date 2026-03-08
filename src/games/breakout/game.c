#include "game.h"
#include "borka_audio.h"
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

  BrTexture *font_atlas = br_texture_create("assets/fonts/font_atlas.png");
  if (!font_atlas) {
    BR_LOG_ERROR("Failed to load font atlas");
    goto error;
  }

  BrFont font = {
      .glyph_size = {8, 8}, .font_atlas = font_atlas, .spacing = {2, 2}};
  game->font = font;

  BrTexture *brick_atlas = br_texture_create("assets/textures/bricks.png");
  if (!brick_atlas) {
    BR_LOG_ERROR("Failed to load brick atlas");
    goto error;
  }

  game->textures.brick_green = (BrTextureRegion){
      .texture = brick_atlas, .position = {0, 0}, .size = {16, 8}};

  game->textures.brick_red = (BrTextureRegion){
      .texture = brick_atlas, .position = {0, 8}, .size = {16, 8}};

  game->textures.brick_blue = (BrTextureRegion){
      .texture = brick_atlas, .position = {0, 16}, .size = {16, 8}};

  game->animations.brick_green_break[0] = (BrTextureRegion){
      .texture = brick_atlas, .position = {0, 0}, .size = {16, 8}};

  game->animations.brick_green_break[1] = (BrTextureRegion){
      .texture = brick_atlas, .position = {16, 0}, .size = {16, 8}};

  game->animations.brick_green_break[2] = (BrTextureRegion){
      .texture = brick_atlas, .position = {32, 0}, .size = {16, 8}};

  game->animations.brick_red_break[0] = (BrTextureRegion){
      .texture = brick_atlas, .position = {0, 8}, .size = {16, 8}};

  game->animations.brick_red_break[1] = (BrTextureRegion){
      .texture = brick_atlas, .position = {16, 8}, .size = {16, 8}};

  game->animations.brick_red_break[2] = (BrTextureRegion){
      .texture = brick_atlas, .position = {32, 8}, .size = {16, 8}};

  game->animations.brick_blue_break[0] = (BrTextureRegion){
      .texture = brick_atlas, .position = {0, 16}, .size = {16, 8}};

  game->animations.brick_blue_break[1] = (BrTextureRegion){
      .texture = brick_atlas, .position = {16, 16}, .size = {16, 8}};

  game->animations.brick_blue_break[2] = (BrTextureRegion){
      .texture = brick_atlas, .position = {32, 16}, .size = {16, 8}};

  game->sfx.bounce_sound = br_sound_create("assets/sfx/bounce.wav");
  if (!game->sfx.bounce_sound) {
    BR_LOG_ERROR("Failed to load bounce sfx");
    goto error;
  }

  game->is_paused = false;
  game->enemies_alive = 0;
  game->game_over = true;
  game->level = 99;

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
  if (game->font.font_atlas) {
    br_texture_destroy(game->font.font_atlas);
  }
  if (game->textures.brick_green.texture) {
    br_texture_destroy(game->textures.brick_green.texture);
  }
  if (game->sfx.bounce_sound)
    br_sound_destroy(game->sfx.bounce_sound);
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
  system_animation(game->app->registry, delta_time);
  system_render(game->app->registry, game->app->renderer);
}
