#ifndef WAYLAND_SHM_H
#define WAYLAND_SHM_H

#include <stdbool.h>
#include <wayland-client.h>

typedef struct {
  struct wl_buffer *wl_buffers[2];
  void *buffer_data[2];
  bool buffer_busy[2];
  void *shared_data;
  size_t buffer_size;
} ShmBufferPair;

ShmBufferPair *wayland_shm_buffer_pair_create(struct wl_shm *shm, int width,
                                              int height);
void wayland_shm_buffer_pair_destroy(ShmBufferPair *pair);

#endif // WAYLAND_SHM_H
