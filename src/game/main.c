#include "borka.h"
#include "borka_texture.h"
#include <stdbool.h>

int main() {
  BrApp *app = br_app_create("Breakout", 200, 200);
  bool running = true;

  // Quad
  BrVec2 q0 = {50, 50};
  BrVec2 q1 = {150, 50};
  BrVec2 q2 = {150, 150};
  BrVec2 q3 = {50, 150};

  // Triangle
  BrVec2 t0 = {100, 50};
  BrVec2 t1 = {150, 150};
  BrVec2 t2 = {50, 150};

  // Ball Texture
  BrTexture *ball_text = br_texture_create("assets/textures/ball.png");

  BrEvent e;

  while (running) {
    // Poll for events
    while (br_window_poll_events(app->window, &e)) {
      switch (e.type) {
      case BR_EVENT_WINDOW_CLOSE:
        running = false;
        break;

      case BR_EVENT_WINDOW_RESIZE:
        br_renderer_resize(app->renderer, e.data.resize.width,
                           e.data.resize.height);
        break;

      case BR_EVENT_KEY_PRESSED:
        BR_LOG_DEBUG("Key %d pressed", e.data.keycode);
        break;

      case BR_EVENT_KEY_RELEASED:
        BR_LOG_DEBUG("Key %d released", e.data.keycode);
        break;
      }
    }

    // Render
    br_renderer_clear(app->renderer, 0xFF000000);
    br_renderer_draw_quad(app->renderer, q0, q1, q2, q3, 0xFF0000FF);
    br_renderer_draw_triangle(app->renderer, t0, t1, t2, 0xFFFFFF00);
    br_renderer_draw_texture(app->renderer, 300, 300, ball_text);
    br_renderer_present(app->renderer);
  }

  br_texture_destroy(ball_text);
  br_app_destroy(app);
  return 0;
}
