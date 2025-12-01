#include "borka.h"
#include "borka_log.h"
#include "logger/br_logger.h"
#include "renderer/br_renderer.h"
#include "window/br_window.h"
#include <stddef.h>
#include <stdlib.h>

static void app_cleanup(BrApp *app) {
  if (!app) {
    return;
  }
  if (app->registry) {
    free(app->registry);
  }
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
    BR_LOG_ERROR("Can't create app without title");
    return NULL;
  }

  if (!br_logger_init(title)) {
    BR_LOG_ERROR("Failed to initialize logging system");
    return NULL;
  }

  BrApp *app = calloc(1, sizeof(BrApp));
  if (!app) {
    BR_LOG_ERROR("Failed to allocate app");
    goto error;
  }
  app->should_shutdown = false;

  BrRegistry *registry = calloc(1, sizeof(BrRegistry));
  if (!registry) {
    BR_LOG_ERROR("Failed to allocate registry");
    goto error;
  }
  app->registry = registry;

  app->window = br_window_create(title, width, height);
  if (!app->window) {
    BR_LOG_ERROR("Failed to create window");
    goto error;
  }

  app->renderer = br_renderer_create(app->window);
  if (!app->renderer) {
    BR_LOG_ERROR("Failed to create renderer");
    goto error;
  }

  return app;

error:
  app_cleanup(app);
  return NULL;
}

void br_app_destroy(BrApp *app) { app_cleanup(app); }
