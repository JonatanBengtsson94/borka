#ifndef WAYLAND_SOFTWARE_RENDERER_H
#define WAYLAND_SOFTWARE_RENDERER_H

#include <stdint.h>

struct BrRenderer {
  uint32_t *pixels;
  struct wl_shm *wl_shm;
  struct wl_surface *wl_surface;
  int width;
  int height;
};

#endif // WAYLAND_SOFTWARE_RENDERER_H
