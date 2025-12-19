#include "pch.h"

#include "borka_events.h"
#include "borka_log.h"
#include "event/br_event.h"
#include "wayland_window.h"
#include "window/br_window.h"
#include "xdg-shell-client-protocol.h"
#include <poll.h>
#include <wayland-client-protocol.h>
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

  if (width <= 0 || height <= 0) {
    BR_LOG_ERROR("xdg toplevel configure with zero or less dimensions");
    return;
  }

  if (window->width == width && window->height == height) {
    return;
  }

  BR_LOG_DEBUG("Window resizing: %dx%d -> %dx%d", window->width, window->height,
               width, height);

  window->width = width;
  window->height = height;

  BrEvent e = {.type = BR_EVENT_WINDOW_RESIZE,
               .data.resize = {.width = width, .height = height}};

  br_event_push(&e);
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  (void)toplevel;
  (void)data;

  BrEvent e = {.type = BR_EVENT_WINDOW_CLOSE};
  BR_LOG_DEBUG("Window close event registered");

  br_event_push(&e);
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

// --- INPUT ---

void keyboard_keymap(void *data, struct wl_keyboard *kb, uint32_t format,
                     int32_t fd, uint32_t size) {
  (void)data;
  (void)kb;
  (void)format;
  (void)fd;
  (void)size;
}

void keyboard_enter(void *data, struct wl_keyboard *kb, uint32_t serial,
                    struct wl_surface *surface, struct wl_array *keys) {
  (void)data;
  (void)kb;
  (void)serial;
  (void)surface;
  (void)keys;
}

void keyboard_leave(void *data, struct wl_keyboard *kb, uint32_t serial,
                    struct wl_surface *surface) {

  (void)data;
  (void)kb;
  (void)serial;
  (void)surface;
}

void keyboard_key(void *data, struct wl_keyboard *kb, uint32_t serial,
                  uint32_t time, uint32_t key, uint32_t state) {
  (void)data;
  (void)kb;
  (void)serial;
  (void)time;

  BrEvent e;

  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    e.type = BR_EVENT_KEY_PRESSED;
    e.data.keycode = key;
  } else {
    e.type = BR_EVENT_KEY_RELEASED;
    e.data.keycode = key;
  }

  br_event_push(&e);
}

void keyboard_modifiers(void *data, struct wl_keyboard *kb, uint32_t serial,
                        uint32_t mods_depressed, uint32_t mods_latched,
                        uint32_t mods_locked, uint32_t group) {
  (void)data;
  (void)kb;
  (void)serial;
  (void)mods_depressed;
  (void)mods_latched;
  (void)mods_locked;
  (void)group;
}

void keyboard_repeat_info(void *data, struct wl_keyboard *kb, int32_t rate,
                          int32_t delay) {
  (void)data;
  (void)kb;
  (void)rate;
  (void)delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};

void seat_capabilities(void *data, struct wl_seat *seat,
                       uint32_t capabilities) {
  BrWindow *window = data;

  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    BR_LOG_DEBUG("Keyboard capability detected");
    window->wl_keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(window->wl_keyboard, &keyboard_listener, window);
  }
}

void seat_name(void *data, struct wl_seat *seat, const char *name) {
  (void)data;
  (void)seat;
  (void)name;
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
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
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    window->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    window->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
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
  if (window->wl_keyboard) {
    wl_keyboard_destroy(window->wl_keyboard);
  }
  if (window->wl_seat) {
    wl_seat_destroy(window->wl_seat);
  }
  if (window->wl_display) {
    wl_display_disconnect(window->wl_display);
  }

  free(window);
}

// --- Public API ---

BrWindow *br_window_create(const char *title, int width, int height) {
  BrWindow *window = calloc(1, sizeof(BrWindow));
  if (!window) {
    return NULL;
  }

  window->height = height;
  window->width = width;
  window->title = title;

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
  BR_LOG_TRACE("Retrieved Wayland registry");

  wl_registry_add_listener(window->wl_registry, &registry_listener, window);
  wl_display_roundtrip(window->wl_display);

  if (!window->wl_compositor) {
    BR_LOG_ERROR("Failed to bind to wl_compositor");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_TRACE("Bound to Wayland compositor");

  if (!window->xdg_wm_base) {
    BR_LOG_ERROR("Failed to bind to xdg_base");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_TRACE("Bound to xdg");

  xdg_wm_base_add_listener(window->xdg_wm_base, &xdg_wm_base_listener, window);
  wl_seat_add_listener(window->wl_seat, &seat_listener, window);

  window->wl_surface = wl_compositor_create_surface(window->wl_compositor);
  if (!window->wl_surface) {
    BR_LOG_ERROR("Failed to create Wayland surface");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_TRACE("Created Wayland surface");

  window->xdg_surface =
      xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->wl_surface);
  if (!window->xdg_surface) {
    BR_LOG_ERROR("Failed to retrieve xdg surface");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_TRACE("Retrieved xdg surface");

  xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (!window->xdg_toplevel) {
    BR_LOG_ERROR("Failed to retrieve xdg top level");
    window_cleanup(window);
    return NULL;
  }
  BR_LOG_TRACE("Retrieved xdg top level");

  xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener,
                            window);
  xdg_toplevel_set_title(window->xdg_toplevel, window->title);
  xdg_toplevel_set_min_size(window->xdg_toplevel, window->width,
                            window->height);

  wl_surface_commit(window->wl_surface);

  BR_LOG_INFO("Window instance created");
  return window;
}

void br_window_destroy(BrWindow *window) {
  BR_LOG_INFO("Window instance destroyed");
  if (!window) {
    return;
  }
  window_cleanup(window);
}

bool br_window_poll_events(BrWindow *window, BrEvent *out_event) {
  if (!window) {
    BR_LOG_ERROR("Can't poll NULL window");
    return false;
  }

  if (!window->wl_display) {
    BR_LOG_ERROR("wl display was NULL");
    return false;
  }

  while (wl_display_prepare_read(window->wl_display) != 0) {
    wl_display_dispatch_pending(window->wl_display);
  }

  wl_display_flush(window->wl_display);

  struct pollfd pfd = {.fd = wl_display_get_fd(window->wl_display),
                       .events = POLLIN};

  if (poll(&pfd, 1, 0) > 0) {
    wl_display_read_events(window->wl_display);
  } else {
    wl_display_cancel_read(window->wl_display);
  }

  wl_display_dispatch_pending(window->wl_display);

  return br_event_poll(out_event);
}
