#ifndef BR_WINDOW_EVENT_H
#define BR_WINDOW_EVENT_H

#include <stdbool.h>

/**
 * @brief Describes the type of window event.
 */
typedef enum {
  BR_WINDOW_EVENT_CLOSE, /**< The window has been requested to close. */
  BR_WINDOW_EVENT_RESIZE /**< The window size has changed. */
} BrWindowEventType;

/**
 * @brief Stores a single window event.
 *
 * The union contains event-specific data. Only fields corresponding to the
 * value of @ref type are valid.
 */
typedef struct {
  BrWindowEventType type; /**< The type of window event. */
  union {
    /** Data for BR_WINDOW_EVENT_RESIZE. */
    struct {
      int width;  /**< The new width of the window in pixels. */
      int height; /**< The new height of the window in pixels. */
    } resize;
  };
} BrWindowEvent;

/**
 * @brief Polls the next available window event.
 *
 * If an event is available, it is written to the out_event.
 *
 * @param out_event Pointer to write the next event into.
 * @return true if an event was returned, false if there was no available event.
 */
bool br_window_event_poll(BrWindowEvent *out_event);

/**
 * @brief Pushes a window event into the event system.
 *
 * @param event The event to push.
 * @return true if the event was added, false if if failed.
 */
bool br_window_event_push(const BrWindowEvent *event);

#endif // BR_WINDOW_EVENT_H
