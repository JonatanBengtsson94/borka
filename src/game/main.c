#define _POSIX_C_SOURCE 199309L

#include "borka.h"
#include "components/components.h"
#include "systems/systems.h"
#include <time.h>

typedef struct {
  BrTexture *paddle_texture;
  BrTexture *ball_texture;
} GameResources;

bool setup(BrRegistry *registry, GameResources *resources) {
  if (!components_register(registry)) {
    BR_LOG_ERROR("Failed to register components");
    return false;
  }

  // Paddle
  resources->paddle_texture = br_texture_create("assets/textures/paddle.png");
  if (!resources->paddle_texture) {
    BR_LOG_ERROR("Failed to create paddle texture");
    return false;
  }
  BrEntity paddle = br_entity_create(registry);
  Position paddle_pos = {100, 190};
  Velocity paddle_vel = {0, 0};
  Sprite paddle_sprite = {resources->paddle_texture};
  InputControlled paddle_input_control = {false, false};
  MovementConfig paddle_movement_conf = {50};
  br_component_add(registry, paddle, COMPONENT_POSITION, &paddle_pos);
  br_component_add(registry, paddle, COMPONENT_VELOCITY, &paddle_vel);
  br_component_add(registry, paddle, COMPONENT_SPRITE, &paddle_sprite);
  br_component_add(registry, paddle, COMPONENT_INPUT_CONTROLLED,
                   &paddle_input_control);
  br_component_add(registry, paddle, COMPONENT_MOVEMENT_CONFIG,
                   &paddle_movement_conf);

  // Ball
  resources->ball_texture = br_texture_create("assets/textures/ball.png");
  if (!resources->ball_texture) {
    BR_LOG_ERROR("Failed to create ball texture");
    return false;
  }
  BrEntity ball = br_entity_create(registry);
  Position ball_pos = {100, 100};
  Velocity ball_vel = {0, 50};
  Sprite ball_sprite = {resources->ball_texture};
  br_component_add(registry, ball, COMPONENT_POSITION, &ball_pos);
  br_component_add(registry, ball, COMPONENT_VELOCITY, &ball_vel);
  br_component_add(registry, ball, COMPONENT_SPRITE, &ball_sprite);

  return true;
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
    case BR_EVENT_KEY_RELEASED:
      system_input(app->registry, e);
      break;
    }
  }
}

void update(BrRegistry *registry, double delta_time) {
  system_player_movement(registry);
  system_physics(registry, delta_time);
}

void render(BrRegistry *registry, BrRenderer *renderer) {
  system_render(registry, renderer);
}

int main() {
  BrApp *app = br_app_create("Breakout", 200, 200);
  GameResources resources = {0};

  if (!setup(app->registry, &resources)) {
    goto cleanup;
  }

  double last = get_time();

  while (!app->should_shutdown) {
    double now = get_time();
    double delta_time = now - last;
    last = now;

    input(app);
    update(app->registry, delta_time);
    render(app->registry, app->renderer);
  }

cleanup:
  if (resources.paddle_texture) {
    br_texture_destroy(resources.paddle_texture);
  }
  if (resources.ball_texture) {
    br_texture_destroy(resources.ball_texture);
  }
  br_app_destroy(app);
  return 0;
}
