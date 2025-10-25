#include <stdbool.h>

struct BrWindow {
  int width;
  int height;
  const char *title;
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct wl_surface *wl_surface;
  struct wl_shm *wl_shm;
  struct xdg_wm_base *xdg_wm_base;
  struct xdg_toplevel *xdg_toplevel;
  struct xdg_surface *xdg_surface;
};
