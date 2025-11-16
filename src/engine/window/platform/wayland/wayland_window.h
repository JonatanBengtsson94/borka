#ifndef WAYLAND_WINDOW_H
#define WAYLAND_WINDOW_H

#include <stdbool.h>

struct BrWindow {
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct wl_surface *wl_surface;
  struct wl_shm *wl_shm;
  struct wl_seat *wl_seat;
  struct wl_keyboard *wl_keyboard;
  struct xdg_wm_base *xdg_wm_base;
  struct xdg_toplevel *xdg_toplevel;
  struct xdg_surface *xdg_surface;
  const char *title;
  int width;
  int height;
};

#endif // WAYLAND_WINDOW_H
