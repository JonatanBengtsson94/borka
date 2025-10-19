#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

/**
 * @brief Opaque handle to platform-independent window.
 */
typedef struct BrWindow BrWindow;

/**
 * @brief Properties used to create a new window.
 */
typedef struct {
  int width;         /**< Width of the window in pixels. */
  int height;        /**< Height of the window in pixels. */
  const char *title; /**< Window title string. */
} BrWindowProps;

/**
 * @brief Creates a new window with the specified properties.
 *
 * @param props Pointer to a BrWindowProps struct defining the window
 * parameters.
 * @return Pointer to a newly created BrWindow instance, or NULL on failure.
 */
BrWindow *br_window_create(const BrWindowProps *props);

/**
 * @brief Destroys a previously created window and releases associated
 * resources.
 *
 * @param window Pointer to the window that should be destroyed.
 */
void br_window_destroy(BrWindow *window);

/**
 * @brief Polls and processes window events without blocking.
 *
 * This function checks for events from the window system (such as resize,
 * input, or close request) and dispatches them to appropriate handlers.
 *
 * @param window Pointer to the window to poll events for.
 * @return true if successful, false if an error occured or the connection was
 * lost.
 */
bool br_window_poll_events(BrWindow *window);

#endif // WINDOW_H
