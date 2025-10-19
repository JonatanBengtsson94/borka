#include "logger.h"
#include "render/render.h"
#include "window/platform/wayland/wayland_shm.h"
#include "window/platform/wayland/wayland_window.h"
#include "window/window.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-client.h>

struct BrRenderer {
  uint32_t *pixels;
  struct wl_shm *wl_shm;
  struct wl_surface *wl_surface;
  int width;
  int height;
};

BrRenderer *wayland_software_renderer_create(BrWindow *window) {
  BrRenderer *renderer = calloc(1, sizeof(BrRenderer));
  if (!renderer) {
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

  BR_LOG_INFO("Software renderer created");
  return renderer;
}

void wayland_software_renderer_destroy(BrRenderer *renderer) { free(renderer); }

void wayland_software_renderer_present(BrRenderer *renderer) {
  ShmBuffer *shm_buf = wayland_shm_buffer_create(
      renderer->wl_shm, renderer->width, renderer->height);

  // Draw a white rectangle
  uint32_t *pixels = (uint32_t *)shm_buf->data;
  for (int y = 0; y < renderer->height; ++y) {
    for (int x = 0; x < renderer->width; ++x) {
      pixels[y * renderer->width + x] = 0xFFFFFFFF; // White pixel (XRGB8888)
    }
  }

  wl_surface_attach(renderer->wl_surface, shm_buf->buffer, 0, 0);
  wl_surface_damage_buffer(renderer->wl_surface, 0, 0, renderer->width,
                           renderer->height);
  wl_surface_commit(renderer->wl_surface);

  wayland_shm_buffer_destroy(shm_buf);
}
