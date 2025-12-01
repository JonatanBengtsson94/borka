#define _POSIX_C_SOURCE 199309L

#include "borka.h"
#include <time.h>

void setup(BrRegistry *registry) {
  BrTexture *ball_text = br_texture_create("assets/textures/ball.png");
  BrTexture *paddle_text = br_texture_create("assets/textures/paddle.png");

  BrEntity ball = create_entity(registry);
  registry->positions[ball] = (BrPosition){100, 100};
  registry->velocities[ball] = (BrVelocity){0, 50};
  registry->sprites[ball] = (BrSprite){ball_text};
  registry->masks[ball] =
      COMPONENT_POSITION | COMPONENT_VELOCITY | COMPONENT_SPRITE;

  BrEntity paddle = create_entity(registry);
  registry->positions[paddle] = (BrPosition){100, 190};
  registry->sprites[paddle] = (BrSprite){paddle_text};
  registry->masks[paddle] = COMPONENT_POSITION | COMPONENT_SPRITE;
}

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

void update(BrRegistry *registry, double delta_time) {
  system_movement(registry, delta_time);
}

void render(BrRegistry *registry, BrRenderer *renderer) {
  system_render(registry, renderer);
}

void shutdown(BrApp *app) { br_app_destroy(app); }

int main() {
  BrApp *app = br_app_create("Breakout", 200, 200);

  setup(app->registry);

  double last = get_time();

  while (!app->should_shutdown) {
    double now = get_time();
    double delta_time = now - last;
    last = now;

    input(app);
    update(app->registry, delta_time);
    render(app->registry, app->renderer);
  }
  return 0;
}
