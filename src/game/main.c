#define _POSIX_C_SOURCE 199309L

#include "borka.h"
#include "borka_texture.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  float x;
  float y;
} Position;

double get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
}

void input(BrApp *app) {
  BrEvent e;
  while (br_window_poll_events(app->window, &e)) {
    switch (e.type) {
    case BR_EVENT_WINDOW_CLOSE:
      app->should_shutdown = true;
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
}

void update(Position *positions, double delta_time) {
  positions[0].y += 50 * delta_time;
  BR_LOG_DEBUG("DT: %f", delta_time);
  BR_LOG_DEBUG("Y: %f", positions[0].y);
}

void render(BrTexture *textures, Position *positions, size_t count,
            BrApp *app) {
  br_renderer_clear(app->renderer, 0xFF000000);
  for (size_t i = 0; i < count; i++) {
    br_renderer_draw_texture(app->renderer, positions[i].x, positions[i].y,
                             &textures[i]);
  }
  br_renderer_present(app->renderer);
}

void shutdown(BrApp *app) { br_app_destroy(app); }

int main() {
  BrApp *app = br_app_create("Breakout", 200, 200);

  BrTexture *ball_text = br_texture_create("assets/textures/ball.png");
  BrTexture *paddle_text = br_texture_create("assets/textures/paddle.png");
  Position ball_position = {.x = 100, .y = 100};
  Position paddle_position = {.x = 100, .y = 190};

  BrTexture textures[2] = {*ball_text, *paddle_text};
  Position positions[2] = {ball_position, paddle_position};

  double last = get_time();

  while (!app->should_shutdown) {
    double now = get_time();
    double delta_time = now - last;
    last = now;

    input(app);
    update(positions, delta_time);
    render(textures, positions, 2, app);
  }

  return 0;
}
