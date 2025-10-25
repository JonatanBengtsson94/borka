#include "wayland_shm.h"
#include "logger/logger.h"
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
  wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener buffer_listener = {
    .release = wl_buffer_release,
};

ShmBuffer *wayland_shm_buffer_create(struct wl_shm *shm, int width,
                                     int height) {
  int stride = width * 4;
  int size = stride * height;
  int fd = create_shm_file(size);
  if (fd < 0) {
    BR_LOG_ERROR("Failed to create shm");
    return NULL;
  }

  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    BR_LOG_ERROR("Failed to mmap shm file");
    close(fd);
    return NULL;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_buffer *buffer = wl_shm_pool_create_buffer(
      pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
  wl_buffer_add_listener(buffer, &buffer_listener, NULL);

  wl_shm_pool_destroy(pool);
  close(fd);

  ShmBuffer *shm_buf = malloc(sizeof(ShmBuffer));
  if (!shm_buf) {
    BR_LOG_ERROR("Failed to create shm buffer");
  }

  shm_buf->buffer = buffer;
  shm_buf->data = data;
  shm_buf->size = size;

  return shm_buf;
}

void wayland_shm_buffer_destroy(ShmBuffer *shm_buf) {
  munmap(shm_buf->data, shm_buf->size);
  wl_buffer_destroy(shm_buf->buffer);
  free(shm_buf);
}
