#define _POSIX_C_SOURCE 200809L

#include "wayland_shm.h"
#include "borka_log.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int create_shm_file(size_t size) {
  int retries = 100;
  char name[] = "/wl_shm-XXXXXX";

  while (retries--) {
    for (int i = 0; i < 6; i++) {
      name[i + 8] = 'A' + rand() % 26;
    }
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
      }

      return fd;
    }
  }

  return -1;
}

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer) {
  (void)wl_buffer;

  bool *busy_flag = data;
  if (busy_flag) {
    *busy_flag = false;
  }
}

static const struct wl_buffer_listener buffer_listener = {
    .release = wl_buffer_release,
};

ShmBufferPair *wayland_shm_buffer_pair_create(struct wl_shm *shm, int width,
                                              int height) {
  int stride = width * 4;
  int buffer_size = stride * height;
  int total_size = buffer_size * 2;

  int fd = create_shm_file(total_size);
  if (fd < 0) {
    BR_LOG_ERROR("Failed to create shm");
    return NULL;
  }

  void *data =
      mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    BR_LOG_ERROR("Failed to mmap shm file");
    close(fd);
    return NULL;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, total_size);
  if (!pool) {
    BR_LOG_ERROR("Failed to create shm_pool");
    munmap(data, total_size);
    close(fd);
    return NULL;
  }

  ShmBufferPair *pair = malloc(sizeof(ShmBufferPair));
  if (!pair) {
    BR_LOG_ERROR("Failed to allocated buffer pair");
    munmap(data, total_size);
    wl_shm_pool_destroy(pool);
    close(fd);
    return NULL;
  }

  pair->buffer_busy[0] = false;
  pair->buffer_busy[1] = false;

  pair->shared_data = data;
  pair->buffer_size = buffer_size;

  pair->buffer_data[0] = data;
  pair->wl_buffers[0] = wl_shm_pool_create_buffer(
      pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_buffer_add_listener(pair->wl_buffers[0], &buffer_listener,
                         &pair->buffer_busy[0]);

  pair->buffer_data[1] = (uint8_t *)data + buffer_size;
  pair->wl_buffers[1] = wl_shm_pool_create_buffer(
      pool, buffer_size, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_buffer_add_listener(pair->wl_buffers[1], &buffer_listener,
                         &pair->buffer_busy[1]);

  wl_shm_pool_destroy(pool);
  close(fd);

  BR_LOG_DEBUG("Created double buffer pair");
  return pair;
}

void wayland_shm_buffer_pair_destroy(ShmBufferPair *pair) {
  if (!pair) {
    return;
  }

  if (pair->wl_buffers[0]) {
    wl_buffer_destroy(pair->wl_buffers[0]);
  }

  if (pair->wl_buffers[1]) {
    wl_buffer_destroy(pair->wl_buffers[1]);
  }

  if (pair->shared_data) {
    munmap(pair->shared_data, pair->buffer_size * 2);
  }

  free(pair);
}
