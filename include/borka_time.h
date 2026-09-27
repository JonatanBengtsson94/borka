#ifndef BORKA_TIME_H
#define BORKA_TIME_H

#include <assert.h>
#include <time.h>

/**
 * @brief Retrieves the current high-resolution time in seconds.
 *
 * Typically used to get the delta_time
 */
static inline double br_time_get() {
#ifdef __unix__
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
#endif
}

/**
 * @brief Sleeps the calling thread for a number of nanoseconds.
 *
 * @param time Time to sleep in nanoseconds. Must be under one second
 * (1000000000); split longer waits into several calls.
 */
static inline void br_time_sleep(int time) {
  // nanosleep() rejects a tv_nsec of a second or more.
  assert(time >= 0 && time < 1000000000);
#ifdef __unix__
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = time;
  nanosleep(&ts, NULL);
#endif
}

#endif // BORKA_TIME_H
