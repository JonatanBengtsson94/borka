#include "borka.h"
#include "logger/br_logger.h"
#include "renderer/br_renderer.h"
#include <stddef.h>
#include <stdlib.h>

static void app_cleanup(BrApp *app) {
  if (app->renderer) {
    br_renderer_destroy(app->renderer);
  }
  if (app->window) {
    br_window_destroy(app->window);
  }
  br_logger_shutdown();
  free(app);
}

BrApp *br_app_create(const char *title, int width, int height) {
  if (!title) {
    return NULL;
  }

  if (!br_logger_init(title)) {
    return NULL;
  }

  BrApp *app = calloc(1, sizeof(BrApp));
  if (!app) {
    BR_LOG_ERROR("Failed to allocate app");
    return NULL;
  }

  app->window = br_window_create(title, width, height);
  if (!app->window) {
    BR_LOG_ERROR("Failed to create window");
    app_cleanup(app);
    return NULL;
  }

  app->renderer = br_renderer_create(app->window);
  if (!app->renderer) {
    BR_LOG_ERROR("Failed to create renderer");
    app_cleanup(app);
    return NULL;
  }

  return app;
}

void br_app_destroy(BrApp *app) {
  if (!app) {
    return;
  }
  app_cleanup(app);
}
