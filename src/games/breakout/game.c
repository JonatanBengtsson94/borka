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

  game->textures.background =
      br_texture_create("assets/textures/background.png");
  if (!game->textures.background) {
    BR_LOG_ERROR("Failed to load background texture");
    goto error;
  }

  BrTexture *font_atlas = br_texture_create("assets/fonts/font_atlas.png");
  if (!font_atlas) {
    BR_LOG_ERROR("Failed to load font atlas");
    goto error;
  }

  // Sets font_atlas even on failure, so game_shutdown still frees it.
  if (!br_font_init(&game->font, font_atlas, (BrVec2){8, 8}, (BrVec2){2, 2},
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")) {
    BR_LOG_ERROR("Failed to set up font");
    goto error;
  }

  BrTexture *trail_atlas = br_texture_create("assets/textures/trail.png");
  if (!trail_atlas) {
    BR_LOG_ERROR("Failed to load trail atlas");
    goto error;
  }

  game->textures.trail[0] = (BrTextureRegion){
      .texture = trail_atlas, .position = {0, 0}, .size = {4, 4}};

  game->textures.trail[1] = (BrTextureRegion){
      .texture = trail_atlas, .position = {8, 0}, .size = {2, 2}};

  game->textures.trail[2] = (BrTextureRegion){
      .texture = trail_atlas, .position = {16, 0}, .size = {1, 1}};

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

  game->sfx.paddle_hit = br_sound_create("assets/sfx/paddle.flac");
  if (!game->sfx.paddle_hit) {
    BR_LOG_ERROR("Failed to load paddle sfx");
    goto error;
  }

  game->sfx.wall_hit = br_sound_create("assets/sfx/wall.flac");
  if (!game->sfx.wall_hit) {
    BR_LOG_ERROR("Failed to load wall sfx");
    goto error;
  }

  game->sfx.brick_hit = br_sound_create("assets/sfx/brick.flac");
  if (!game->sfx.brick_hit) {
    BR_LOG_ERROR("Failed to load brick sfx");
    goto error;
  }

  game->music.menu = br_sound_create("assets/music/background.flac");
  if (!game->music.menu) {
    BR_LOG_ERROR("Failed to load menu music");
    goto error;
  }

  game->music.gameplay = br_sound_create("assets/music/gameplay.flac");
  if (!game->music.gameplay) {
    BR_LOG_ERROR("Failed to load gameplay music");
    goto error;
  }

  game->is_paused = false;
  game->enemies_alive = 0;

  scene_load(game, SCENE_START);

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
  if (game->textures.background)
    br_texture_destroy(game->textures.background);
  if (game->font.font_atlas) {
    br_texture_destroy(game->font.font_atlas);
  }
  if (game->textures.brick_green.texture) {
    br_texture_destroy(game->textures.brick_green.texture);
  }
  if (game->textures.trail[0].texture)
    br_texture_destroy(game->textures.trail[0].texture);
  if (game->sfx.paddle_hit)
    br_sound_destroy(game->sfx.paddle_hit);
  if (game->sfx.wall_hit)
    br_sound_destroy(game->sfx.wall_hit);
  if (game->sfx.brick_hit)
    br_sound_destroy(game->sfx.brick_hit);
  if (game->music.menu)
    br_sound_destroy(game->music.menu);
  if (game->music.gameplay)
    br_sound_destroy(game->music.gameplay);
  if (game->app)
    br_app_destroy(game->app);
}

void game_handle_event(GameState *game, BrEvent event) {
  if (event.type != BR_EVENT_KEY_PRESSED &&
      event.type != BR_EVENT_KEY_RELEASED)
    return;

  // Only a press starts a run, so releasing a key still held when the last
  // run ended does not skip the game over screen. The key is not forwarded
  // either, or starting with space would pause the new level. Presses are
  // ignored until the menu shows its prompt, see scene_update().
  bool in_menu = game->scene.id == SCENE_START ||
                 game->scene.id == SCENE_GAME_OVER;
  if (in_menu) {
    if (event.type == BR_EVENT_KEY_PRESSED && game->scene.input_ready)
      scene_load(game, SCENE_LEVEL_01);
    return;
  }

  system_input(game, event);
}

void game_update(GameState *game, double delta_time) {
  scene_update(game, delta_time);
  if (game->is_paused)
    return;
  if (game->game_over && game->scene.id == SCENE_LEVEL_01) {
    scene_load(game, SCENE_GAME_OVER);
    return;
  }
  system_player_movement(game->app->registry);
  system_movement(game->app->registry, delta_time);
  system_collision_detection(game->app->registry);
  system_collision_handling(game);
  system_animation(game->app->registry, delta_time);
  system_trail(game->app->registry);
  system_render(game->app->registry, game->app->renderer, &game->scene);
}
