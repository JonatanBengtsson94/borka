#ifndef WAYLAND_SOFTWARE_RENDERER_H
#define WAYLAND_SOFTWARE_RENDERER_H

#include "window/platform/wayland/wayland_shm.h"
#include <stdint.h>

struct BrRenderer {
  struct wl_shm *wl_shm;
  struct wl_surface *wl_surface;
  struct wl_display *wl_display;
  int width;
  int height;
  ShmBufferPair *buffers;
  int front_buffer_index;
  int back_buffer_index;
};

#endif // WAYLAND_SOFTWARE_RENDERER_H
