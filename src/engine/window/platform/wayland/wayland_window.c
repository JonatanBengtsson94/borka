#include "wayland_window.h"
#include "logger/br_logger.h"
#include "window/br_window.h"
#include "xdg-shell-client-protocol.h"
#include <poll.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

// --- XDG SETUP ---

static void xdg_surface_configure(void *data, struct xdg_surface *surface,
                                  uint32_t serial) {
  (void)data;

  xdg_surface_ack_configure(surface, serial);
}

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                   int32_t width, int32_t height,
                                   struct wl_array *states) {
  (void)toplevel;
  (void)states;

  BrWindow *window = data;
  if (width > 0 && height > 0) {
    window->width = width;
    window->height = height;
  }
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  (void)toplevel;

  BrWindow *window = data;
  BR_LOG_INFO("Recieved close event from compositor");
  window->should_close = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  (void)data;

  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

// --- WAYLAND REGISTRY SETUP ---

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  (void)version;

  BrWindow *window = data;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    window->wl_compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    window->xdg_wm_base =
        wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(window->xdg_wm_base, &xdg_wm_base_listener,
                             window);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    window->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
  }
}

static void registry_handle_global_remove(void *data,
                                          struct wl_registry *registry,
                                          uint32_t name) {
  (void)data;
  (void)registry;
  (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

// --- INTERNAL UTILITIES ---

static void window_cleanup(BrWindow *window) {
  if (window->xdg_toplevel) {
    xdg_toplevel_destroy(window->xdg_toplevel);
  }
  if (window->xdg_surface) {
    xdg_surface_destroy(window->xdg_surface);
  }
  if (window->wl_surface) {
    wl_surface_destroy(window->wl_surface);
  }
  if (window->wl_compositor) {
    wl_compositor_destroy(window->wl_compositor);
  }
  if (window->xdg_wm_base) {
    xdg_wm_base_destroy(window->xdg_wm_base);
  }
  if (window->wl_shm) {
    wl_shm_destroy(window->wl_shm);
  }
  if (window->wl_registry) {
    wl_registry_destroy(window->wl_registry);
  }
  if (window->wl_display) {
    wl_display_disconnect(window->wl_display);
  }

  free(window);
}

// --- Public API ---

BrWindow *br_window_create(const BrWindowProps *props) {
  if (!props) {
    return NULL;
  }

  BrWindow *window = calloc(1, sizeof(BrWindow));
  if (!window) {
    return NULL;
  }
  window->height = props->height;
  window->width = props->width;
  window->title = props->title;

  window->wl_display = wl_display_connect(NULL);
  if (!window->wl_display) {
    BR_LOG_ERROR("Failed to connect to Wayland display");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_INFO("Connection to Wayland display established");

  window->wl_registry = wl_display_get_registry(window->wl_display);
  if (!window->wl_registry) {
    BR_LOG_ERROR("Failed to get Wayland registry");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_DEBUG("Retrieved Wayland registry");

  wl_registry_add_listener(window->wl_registry, &registry_listener, window);
  wl_display_roundtrip(window->wl_display);

  if (!window->wl_compositor) {
    BR_LOG_ERROR("Failed to bind to wl_compositor");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_DEBUG("Bound to Wayland compositor");

  if (!window->xdg_wm_base) {
    BR_LOG_ERROR("Failed to bind to xdg_base");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_DEBUG("Bound to xdg");

  window->wl_surface = wl_compositor_create_surface(window->wl_compositor);
  if (!window->wl_surface) {
    BR_LOG_ERROR("Failed to create Wayland surface");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_DEBUG("Created Wayland surface");

  window->xdg_surface =
      xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->wl_surface);
  if (!window->xdg_surface) {
    BR_LOG_ERROR("Failed to retrieve xdg surface");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_DEBUG("Retrieved xdg surface");

  xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (!window->xdg_toplevel) {
    BR_LOG_ERROR("Failed to retrieve xdg top level");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_DEBUG("Retrieved xdg top level");

  xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener,
                            window);
  xdg_toplevel_set_title(window->xdg_toplevel, window->title);
  xdg_toplevel_set_min_size(window->xdg_toplevel, window->width,
                            window->height);

  return window;
}

void br_window_destroy(BrWindow *window) {
  if (!window) {
    return;
  }
  window_cleanup(window);
}

bool br_window_poll_events(BrWindow *window) {
  if (wl_display_dispatch_pending(window->wl_display) == -1) {
    return false;
  }

  if (wl_display_flush(window->wl_display) == -1) {
    return false;
  }

  struct pollfd fds = {
      .fd = wl_display_get_fd(window->wl_display),
      .events = POLLIN,
  };

  if (poll(&fds, 1, 0) > 0) {
    if (wl_display_dispatch(window->wl_display) == -1) {
      return false;
    }
  }

  if (window->should_close) {
    return false;
  }

  return true;
}
