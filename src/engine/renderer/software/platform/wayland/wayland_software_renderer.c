#include "renderer/software/platform/wayland/wayland_software_renderer.h"
#include "borka_log.h"
#include "borka_math.h"
#include "renderer/software/br_software_renderer.h"
#include "wayland_software_renderer.h"
#include "window/platform/wayland/wayland_shm.h"
#include "window/platform/wayland/wayland_window.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <wayland-client.h>

static bool on_screen(const struct BrRenderer *renderer, int maxX, int minX,
                      int maxY, int minY) {
  assert(renderer);
  assert(renderer->game_pixels);

  if (minX > renderer->game_width || maxX < 0 || minY > renderer->game_height ||
      maxY < 0)
    return false;

  return true;
}

static bool scale_render_target(const int *source_pixels, int *target_pixels,
                                int source_width, int source_height,
                                int target_width, int target_height) {
  if (!source_pixels) {
    BR_LOG_ERROR("Could not scale render target: source pixels was NULL");
    return false;
  }
  if (!target_pixels) {
    BR_LOG_ERROR("Could not scale render target: target pixels was NULL");
    return false;
  }
  if (source_width <= 0 || source_height <= 0 || target_width <= 0 ||
      target_height <= 0) {
    BR_LOG_ERROR("Could not scale render target, invalid dimenstions");
    return false;
  }

  int scale_factor_x = target_width / source_width;
  int scale_factor_y = target_height / source_height;
  int scale_factor = min_int(scale_factor_x, scale_factor_y);

  if (scale_factor < 1) {
    BR_LOG_WARN("Render target is smaller then game dimensions");
    scale_factor = 1;
  }

  int scaled_width = source_width * scale_factor;
  int scaled_height = source_height * scale_factor;
  int offsetX = (target_width - scaled_width) / 2;
  int offsetY = (target_height - scaled_height) / 2;

  for (int y_source = 0; y_source < source_height; ++y_source) {
    for (int x_source = 0; x_source < source_width; ++x_source) {
      int color = source_pixels[y_source * source_width + x_source];

      int x_target_start = offsetX + x_source * scale_factor;
      int y_target_start = offsetY + y_source * scale_factor;

      for (int dy = 0; dy < scale_factor; ++dy) {
        int y_target = y_target_start + dy;
        int target_row_start = y_target * target_width;

        for (int dx = 0; dx < scale_factor; ++dx) {
          int x_target = x_target_start + dx;
          target_pixels[target_row_start + x_target] = color;
        }
      }
    }
  }

  return true;
}

void br_renderer_destroy(struct BrRenderer *renderer) {
  BR_LOG_INFO("Software renderer destroyed");
  if (!renderer) {
    return;
  }
  if (renderer->buffers) {
    wayland_shm_buffer_pair_destroy(renderer->buffers);
  }
  if (renderer->game_pixels) {
    free(renderer->game_pixels);
  }
  free(renderer);
}

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
  renderer->game_width = window->width;
  renderer->game_height = window->height;

  if (!renderer->wl_shm) {
    BR_LOG_ERROR("shm not available for renderer");
    goto error;
  }

  if (!renderer->wl_surface) {
    BR_LOG_ERROR("surface not available for renderer");
    goto error;
  }

  renderer->game_pixels =
      calloc(renderer->game_width * renderer->game_height, sizeof(int));
  if (!renderer->game_pixels) {
    BR_LOG_ERROR("Failed to allocate game pixel buffer");
    goto error;
  }

  renderer->buffers = wayland_shm_buffer_pair_create(
      renderer->wl_shm, renderer->width, renderer->height);
  renderer->front_buffer_index = 0;
  renderer->back_buffer_index = 1;

  if (!renderer->buffers) {
    BR_LOG_ERROR("Failed to create shm buffers");
    goto error;
  }

  BR_LOG_INFO("Software renderer created");
  return renderer;

error:
  br_renderer_destroy(renderer);
  return NULL;
}

void br_renderer_clear(struct BrRenderer *renderer, int color) {
  assert(renderer);

  software_clear(renderer->game_pixels, renderer->game_width,
                 renderer->game_height, color);
}

void br_renderer_draw_filled_triangle(struct BrRenderer *renderer, BrVec2 v0,
                                      BrVec2 v1, BrVec2 v2, int color) {
  assert(renderer);

  software_draw_filled_triangle(renderer->game_pixels, renderer->game_width,
                                renderer->game_height, v0, v1, v2, color);
}

void br_renderer_draw_rectangle_filled(struct BrRenderer *renderer, int x,
                                       int y, int width, int height,
                                       int color) {
  if (on_screen(renderer, x + width, x, y + height, y))
    software_draw_rectangle_filled(renderer->game_pixels, renderer->game_width,
                                   renderer->game_height, x, y, width, height,
                                   color);
}

void br_renderer_draw_rectangle_outlined(struct BrRenderer *renderer, int x,
                                         int y, int width, int height,
                                         int color) {
  if (on_screen(renderer, x + width, x, y + height, y))
    software_draw_rectangle_outlined(
        renderer->game_pixels, renderer->game_width, renderer->game_height, x,
        y, width, height, color);
}

void br_renderer_draw_texture(struct BrRenderer *renderer, int x, int y,
                              const BrTexture *texture) {
  assert(texture);
  assert(texture->pixels);

  if (on_screen(renderer, x + texture->width, x, y + texture->height, y))
    software_draw_texture(renderer->game_pixels, renderer->game_width,
                          renderer->game_height, x, y, texture);
}

void br_renderer_present(struct BrRenderer *renderer) {
  assert(renderer);

  int back = renderer->back_buffer_index;

  if (renderer->buffers->buffer_busy[back]) {
    BR_LOG_DEBUG("Back buffer is busy, dropping frame");
    return;
  }

  if (!scale_render_target(renderer->game_pixels,
                           renderer->buffers->buffer_data[back],
                           renderer->game_width, renderer->game_height,
                           renderer->width, renderer->height))

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

void br_renderer_resize(struct BrRenderer *renderer, int width, int height) {
  if (!renderer) {
    BR_LOG_ERROR("Cannot resize: renderer is NULL");
    return;
  }

  if (renderer->width == width && renderer->height == height) {
    return;
  }

  BR_LOG_DEBUG("Renderer resizing: %dx%d -> %dx%d", renderer->width,
               renderer->height, width, height);

  if (renderer->buffers) {
    wayland_shm_buffer_pair_destroy(renderer->buffers);
  }

  renderer->width = width;
  renderer->height = height;

  renderer->buffers = wayland_shm_buffer_pair_create(
      renderer->wl_shm, renderer->width, renderer->height);

  if (!renderer->buffers) {
    BR_LOG_ERROR("Failed to recreate buffers during resize");
    return;
  }

  renderer->front_buffer_index = 0;
  renderer->back_buffer_index = 1;
}
