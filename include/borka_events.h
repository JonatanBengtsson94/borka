#ifndef BORKA_EVENTS_H
#define BORKA_EVENTS_H

/**
 * @brief Describes the type of event.
 */
typedef enum {
  BR_EVENT_WINDOW_CLOSE,  /**< The window has been requested to close. */
  BR_EVENT_WINDOW_RESIZE, /**< The window size has changed. */
  BR_EVENT_KEY_PRESSED,   /** A key has been pressed. */
  BR_EVENT_KEY_RELEASED,  /** A key has been released. */
} BrEventType;

/**
 * @brief Stores a single window event.
 *
 * The union contains event-specific data. Only fields corresponding to the
 * value of @ref type are valid.
 */
typedef struct {
  BrEventType type; /**< The type of window event. */

  union {
    /** Data for resize events. */
    struct {
      int width;  /**< The new width of the window in pixels. */
      int height; /**< The new height of the window in pixels. */
    } resize;

    /** Data for key events. */
    int keycode; /**< The key code associated with the event (raw evdev). */

  } data;
} BrEvent;

#endif // BORKA_EVENTS_H
