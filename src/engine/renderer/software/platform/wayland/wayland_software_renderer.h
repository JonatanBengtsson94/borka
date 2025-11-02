#ifndef WAYLAND_SOFTWARE_RENDERER_H
#define WAYLAND_SOFTWARE_RENDERER_H

#include "window/platform/wayland/wayland_shm.h"
#include <stdint.h>

struct BrRenderer {
  struct wl_shm *wl_shm;
  struct wl_surface *wl_surface;
  int width;
  int height;
  ShmBuffer *shm_buf;
};

#endif // WAYLAND_SOFTWARE_RENDERER_H
