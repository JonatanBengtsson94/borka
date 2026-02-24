#include "pch.h"

#include "borka_log.h"
#include "renderer/software/br_software_renderer.h"
#include "renderer/software/platform/wayland/wayland_software_renderer.h"
#include "wayland_software_renderer.h"
#include "window/platform/wayland/wayland_shm.h"
#include "window/platform/wayland/wayland_window.h"
#include <wayland-client.h>

static bool on_screen(const struct BrRenderer *renderer, int maxX, int minX,
                      int maxY, int minY) {
  assert(renderer);
  assert(renderer->game_pixels);

  if (minX > renderer->game_dimensions.x || maxX < 0 ||
      minY > renderer->game_dimensions.y || maxY < 0)
    return false;

  return true;
}

static bool scale_render_target(const int *source_pixels, int *target_pixels,
                                BrVec2 source_size, BrVec2 target_size) {
  if (!source_pixels) {
    BR_LOG_ERROR("Could not scale render target: source pixels was NULL");
    return false;
  }
  if (!target_pixels) {
    BR_LOG_ERROR("Could not scale render target: target pixels was NULL");
    return false;
  }
  if (source_size.x <= 0 || source_size.y <= 0 || target_size.x <= 0 ||
      target_size.y <= 0) {
    BR_LOG_ERROR("Could not scale render target, invalid dimenstions");
    return false;
  }

  int scale_factor_x = target_size.x / source_size.x;
  int scale_factor_y = target_size.y / source_size.y;
  int scale_factor = min_int(scale_factor_x, scale_factor_y);

  if (scale_factor < 1) {
    BR_LOG_WARN("Render target is smaller then game dimensions");
    scale_factor = 1;
  }

  int scaled_width = source_size.x * scale_factor;
  int scaled_height = source_size.y * scale_factor;
  int offsetX = (target_size.x - scaled_width) / 2;
  int offsetY = (target_size.y - scaled_height) / 2;

  for (int y_source = 0; y_source < source_size.y; ++y_source) {
    for (int x_source = 0; x_source < source_size.x; ++x_source) {
      int color = source_pixels[y_source * source_size.x + x_source];

      int x_target_start = offsetX + x_source * scale_factor;
      int y_target_start = offsetY + y_source * scale_factor;

      for (int dy = 0; dy < scale_factor; ++dy) {
        int y_target = y_target_start + dy;
        int target_row_start = y_target * target_size.x;

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
  renderer->dimensions.x = window->width;
  renderer->dimensions.y = window->height;
  renderer->game_dimensions.x = window->width;
  renderer->game_dimensions.y = window->height;

  if (!renderer->wl_shm) {
    BR_LOG_ERROR("shm not available for renderer");
    goto error;
  }

  if (!renderer->wl_surface) {
    BR_LOG_ERROR("surface not available for renderer");
    goto error;
  }

  renderer->game_pixels = calloc(
      renderer->game_dimensions.x * renderer->game_dimensions.y, sizeof(int));
  if (!renderer->game_pixels) {
    BR_LOG_ERROR("Failed to allocate game pixel buffer");
    goto error;
  }

  renderer->buffers = wayland_shm_buffer_pair_create(
      renderer->wl_shm, renderer->game_dimensions.x,
      renderer->game_dimensions.y);
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

  software_clear(renderer->game_pixels, renderer->game_dimensions, color);
}

void br_renderer_draw_rectangle_filled(struct BrRenderer *renderer,
                                       BrVec2 position, BrVec2 size,
                                       int color) {
  if (on_screen(renderer, position.x + size.x, position.x, position.y + size.y,
                position.y))
    software_draw_rectangle_filled(renderer->game_pixels,
                                   renderer->game_dimensions, position, size,
                                   color);
}

void br_renderer_draw_rectangle_outlined(struct BrRenderer *renderer,
                                         BrVec2 position, BrVec2 size,
                                         int color) {
  if (on_screen(renderer, position.x + size.x, position.x, position.y + size.y,
                position.y))
    software_draw_rectangle_outlined(renderer->game_pixels,
                                     renderer->game_dimensions, position, size,
                                     color);
}

void br_renderer_draw_texture(struct BrRenderer *renderer, BrVec2 position,
                              const BrTexture *texture) {
  assert(texture);

  if (on_screen(renderer, position.x + texture->size.x, position.x,
                position.y + texture->size.y, position.y))
    software_draw_texture(renderer->game_pixels, renderer->game_dimensions,
                          position, texture);
}

void br_renderer_draw_texture_region(struct BrRenderer *renderer,
                                     BrVec2 position, BrTextureRegion region) {
  assert(region.texture);

  if (on_screen(renderer, position.x + region.size.x, position.x,
                position.y + region.size.y, position.y))
    software_draw_texture_region(renderer->game_pixels,
                                 renderer->game_dimensions, position, region);
}

void br_renderer_draw_text(struct BrRenderer *renderer, const BrFont *font,
                           const char *text, BrVec2 position) {
  if (on_screen(renderer, position.x + font->font_atlas->size.x, position.x,
                position.y + font->font_atlas->size.y, position.y))
    software_draw_text(renderer->game_pixels, renderer->game_dimensions,
                       position, font, text);
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
                           renderer->game_dimensions, renderer->dimensions))

    renderer->buffers->buffer_busy[back] = true;
  wl_surface_attach(renderer->wl_surface, renderer->buffers->wl_buffers[back],
                    0, 0);
  wl_surface_damage_buffer(renderer->wl_surface, 0, 0, renderer->dimensions.x,
                           renderer->dimensions.y);
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

  if (renderer->dimensions.x == width && renderer->dimensions.y == height) {
    return;
  }

  BR_LOG_DEBUG("Renderer resizing: %dx%d -> %dx%d", renderer->dimensions.x,
               renderer->dimensions.y, width, height);

  if (renderer->buffers) {
    wayland_shm_buffer_pair_destroy(renderer->buffers);
  }

  renderer->dimensions.x = width;
  renderer->dimensions.y = height;

  renderer->buffers = wayland_shm_buffer_pair_create(
      renderer->wl_shm, renderer->dimensions.x, renderer->dimensions.y);

  if (!renderer->buffers) {
    BR_LOG_ERROR("Failed to recreate buffers during resize");
    return;
  }

  renderer->front_buffer_index = 0;
  renderer->back_buffer_index = 1;
}
