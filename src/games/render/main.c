#include "logger/br_logger.h"
#include "renderer/br_renderer.h"
#include "window/br_window.h"

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 180

int main() {
  if (!br_logger_init("Render"))
    return 1;

  BrWindow *window = br_window_create("Render", WINDOW_WIDTH, WINDOW_HEIGHT);
  if (!window) {
    br_logger_shutdown();
    return 1;
  }

  BrRenderer *renderer = br_renderer_create(window);
  if (!renderer) {
    br_window_destroy(window);
    br_logger_shutdown();
    return 1;
  }

  BrTexture *ball = br_texture_create("assets/textures/ball.png");
  BrTexture *paddle = br_texture_create("assets/textures/paddle.png");
  BrTexture *bricks = br_texture_create("assets/textures/bricks.png");
  BrTexture *font_atlas = br_texture_create("assets/fonts/font_atlas.png");
  if (!ball || !paddle || !bricks || !font_atlas) {
    BR_LOG_ERROR("Failed to load one or more test assets");
    br_renderer_destroy(renderer);
    br_window_destroy(window);
    br_logger_shutdown();
    return 1;
  }

  BrTextureRegion brick_red = {
      .texture = bricks, .position = {0, 8}, .size = {16, 8}};
  BrFont font = {
      .glyph_size = {8, 8}, .font_atlas = font_atlas, .spacing = {2, 2}};

  BrVec2 filled_pos = {16, 16};
  BrVec2 filled_size = {48, 48};
  BrVec2 outlined_pos = {80, 16};
  BrVec2 outlined_size = {48, 48};
  BrVec2 ball_pos = {144, 16};
  BrVec2 paddle_pos = {160, 40};
  BrVec2 brick_pos = {192, 16};
  BrVec2 text_pos = {16, 100};

  bool should_shutdown = false;
  BrEvent e;
  while (!should_shutdown) {
    while (br_window_poll_events(window, &e)) {
      switch (e.type) {
      case BR_EVENT_WINDOW_CLOSE:
        should_shutdown = true;
        break;

      case BR_EVENT_WINDOW_RESIZE:
        br_renderer_resize(renderer, e.data.resize.width,
                           e.data.resize.height);
        break;

      default:
        break;
      }
    }

    br_renderer_clear(renderer, 0xFF000000);
    br_renderer_draw_rectangle_filled(renderer, filled_pos, filled_size,
                                      0xFFFFFFFF);
    br_renderer_draw_rectangle_outlined(renderer, outlined_pos, outlined_size,
                                        0xFFFF0000);
    br_renderer_draw_texture(renderer, ball_pos, ball);
    br_renderer_draw_texture(renderer, paddle_pos, paddle);
    br_renderer_draw_texture_region(renderer, brick_pos, brick_red);
    br_renderer_draw_text(renderer, &font, "BORKA SHAPES", text_pos);
    br_renderer_present(renderer);
  }

  br_texture_destroy(ball);
  br_texture_destroy(paddle);
  br_texture_destroy(bricks);
  br_texture_destroy(font_atlas);
  br_renderer_destroy(renderer);
  br_window_destroy(window);
  br_logger_shutdown();
  return 0;
}
