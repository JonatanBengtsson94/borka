#include "renderer/software/platform/wayland/wayland_software_renderer.h"
#include "logger/br_logger.h"
#include "math/br_math.h"
#include "renderer/software/br_software_renderer.h"
#include "wayland_software_renderer.h"
#include "window/platform/wayland/wayland_shm.h"
#include "window/platform/wayland/wayland_window.h"
#include <stddef.h>
#include <stdlib.h>
#include <wayland-client.h>

struct BrRenderer *br_renderer_create(struct BrWindow *window) {
  if (!window) {
    BR_LOG_ERROR("Cannot create renderer with NULL window");
    return NULL;
  }

  struct BrRenderer *renderer = calloc(1, sizeof(struct BrRenderer));
  if (!renderer) {
    BR_LOG_ERROR("Failed to allocate renderer");
    return NULL;
  }

  renderer->wl_shm = window->wl_shm;
  renderer->wl_surface = window->wl_surface;
  renderer->width = window->width;
  renderer->height = window->height;

  if (!renderer->wl_shm) {
    BR_LOG_ERROR("shm not available for renderer");
    free(renderer);
    return NULL;
  }

  if (!renderer->wl_surface) {
    BR_LOG_ERROR("surface not available for renderer");
    free(renderer);
    return NULL;
  }

  renderer->shm_buf = wayland_shm_buffer_create(
      renderer->wl_shm, renderer->width, renderer->height);

  if (!renderer->shm_buf) {
    BR_LOG_ERROR("Failed to create SHM buffer");
    free(renderer);
    return NULL;
  }

  BR_LOG_INFO("Software renderer created");
  return renderer;
}

void br_renderer_destroy(struct BrRenderer *renderer) {
  if (!renderer) {
    return;
  }
  if (renderer->shm_buf) {
    wayland_shm_buffer_destroy(renderer->shm_buf);
    free(renderer);
  }
}

void br_renderer_clear(struct BrRenderer *renderer, int color) {
  if (!renderer) {
    BR_LOG_ERROR("Could not clear: renderer is NULL");
    return;
  }
  software_clear(renderer->shm_buf->data, renderer->width, renderer->height,
                 color);
}

void br_renderer_draw_triangle(struct BrRenderer *renderer, BrVec2 v0,
                               BrVec2 v1, BrVec2 v2, int color) {
  if (!renderer) {
    BR_LOG_ERROR("Could not draw: renderer is NULL");
    return;
  }
  software_draw_triangle(renderer->shm_buf->data, renderer->width,
                         renderer->height, v0, v1, v2, color);
}

void br_renderer_present(struct BrRenderer *renderer) {
  if (!renderer) {
    BR_LOG_ERROR("Cannot present: renderer is NULL");
    return;
  }

  wl_surface_attach(renderer->wl_surface, renderer->shm_buf->buffer, 0, 0);
  wl_surface_damage_buffer(renderer->wl_surface, 0, 0, renderer->width,
                           renderer->height);
  wl_surface_commit(renderer->wl_surface);
}
