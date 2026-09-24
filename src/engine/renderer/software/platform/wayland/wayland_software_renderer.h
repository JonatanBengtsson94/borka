#ifndef WAYLAND_SOFTWARE_RENDERER_H
#define WAYLAND_SOFTWARE_RENDERER_H

#include "borka_math.h"
#include "renderer/br_render_queue.h"
#include "window/platform/wayland/wayland_shm.h"

struct BrRenderer {
  struct wl_shm *wl_shm;
  struct wl_surface *wl_surface;
  struct wl_display *wl_display;
  ShmBufferPair *buffers;
  int *game_pixels;
  BrVec2 dimensions;
  BrVec2 game_dimensions;
  int front_buffer_index;
  int back_buffer_index;
  BrRenderQueue *queue;
};

#endif // WAYLAND_SOFTWARE_RENDERER_H
