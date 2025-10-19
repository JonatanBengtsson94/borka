#ifndef WAYLAND_SHM_H
#define WAYLAND_SHM_H

#include <wayland-client.h>

typedef struct ShmBuffer {
  struct wl_buffer *buffer;
  void *data;
  int size;
} ShmBuffer;

ShmBuffer *wayland_shm_buffer_create(struct wl_shm *shm, int width, int height);
void wayland_shm_buffer_destroy(ShmBuffer *buf);

#endif // WAYLAND_SHM_H
