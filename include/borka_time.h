#ifndef BORKA_TIME_H
#define BORKA_TIME_H

#ifdef __unix__
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#endif

/**
 * @brief Retrieves the current high-resolution time in seconds.
 *
 * Typically used to get the delta_time
 */
static inline double br_get_time() {
#ifdef __unix__
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
#endif
}

#endif // BORKA_TIME_H
