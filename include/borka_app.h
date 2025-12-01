#ifndef BORKA_APP_H
#define BORKA_APP_H

#include "borka_render.h"
#include "borka_window.h"

/**
 * @brief Represents an application instance containing a window and a render.
 *
 * The BrApp structure is a high-level container. It is typically created once
 * per application.
 *
 */
typedef struct BrApp {
  BrWindow *window;     /**< The window instance used by the application. */
  BrRenderer *renderer; /**< The renderer responsible for drawing. */
  bool should_shutdown; /**< Boolean indicating wether the app should shutdown.
                         */
} BrApp;

/**
 * @brief Creates a new BrApp instance.
 *
 * This function initializes the application logger, creates a window, and sets
 * up a renderer associated with that window.
 *
 * @param title The title used for the application window and the logfile dir
 * name. Passing NULL is safe but no instance will be created.
 * @param width The width of the application window in pixels.
 * @param height The height of the application window in pixels.
 *
 * @note Should be destroyed with br_app_destroy() when no longer needed.
 */
BrApp *br_app_create(const char *title, int width, int height);

/**
 * @brief Destroys a BrApp instance and releases all allocated resources.
 *
 * This function cleans up the renderer, window and logger subsystems and frees
 * the memory used bt the BrApp structure itself.
 *
 * @param The BrApp instance to destroy. Passing NULL is safe and does nothing.
 */
void br_app_destroy(BrApp *app);

#endif // BORKA_APP_H
