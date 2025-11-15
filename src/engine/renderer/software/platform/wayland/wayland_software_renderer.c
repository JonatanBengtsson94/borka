#include "renderer/software/platform/wayland/wayland_software_renderer.h"
#include "borka_log.h"
#include "borka_math.h"
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
  renderer->wl_display = window->wl_display;
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

  renderer->buffers = wayland_shm_buffer_pair_create(
      renderer->wl_shm, renderer->width, renderer->height);
  renderer->front_buffer_index = 0;
  renderer->back_buffer_index = 1;

  if (!renderer->buffers) {
    BR_LOG_ERROR("Failed to create shm buffers");
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
  if (renderer->buffers) {
    wayland_shm_buffer_pair_destroy(renderer->buffers);
  }
  free(renderer);
}

void br_renderer_clear(struct BrRenderer *renderer, int color) {
  if (!renderer) {
    BR_LOG_ERROR("Could not clear: renderer is NULL");
    return;
  }

  software_clear(renderer->buffers->buffer_data[renderer->back_buffer_index],
                 renderer->width, renderer->height, color);
}

void br_renderer_draw_triangle(struct BrRenderer *renderer, BrVec2 v0,
                               BrVec2 v1, BrVec2 v2, int color) {
  if (!renderer) {
    BR_LOG_ERROR("Could not draw: renderer is NULL");
    return;
  }

  software_draw_triangle(
      renderer->buffers->buffer_data[renderer->back_buffer_index],
      renderer->width, renderer->height, v0, v1, v2, color);
}

void br_renderer_present(struct BrRenderer *renderer) {
  if (!renderer) {
    BR_LOG_ERROR("Cannot present: renderer is NULL");
    return;
  }

  int back = renderer->back_buffer_index;

  while (renderer->buffers->buffer_busy[back]) {
    BR_LOG_WARN("Back buffer is busy");
    wl_display_dispatch(renderer->wl_display);
  }

  renderer->buffers->buffer_busy[back] = true;
  wl_surface_attach(renderer->wl_surface, renderer->buffers->wl_buffers[back],
                    0, 0);
  wl_surface_damage_buffer(renderer->wl_surface, 0, 0, renderer->width,
                           renderer->height);
  wl_surface_commit(renderer->wl_surface);

  int temp = renderer->front_buffer_index;
  renderer->front_buffer_index = renderer->back_buffer_index;
  renderer->back_buffer_index = temp;
}
