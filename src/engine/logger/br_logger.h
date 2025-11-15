#ifndef BR_LOGGER_H
#define BR_LOGGER_H

#include <stdbool.h>

/**
 * @brief Initializes the logger system.
 *
 * Should be called once at application startup before any log messages.
 *
 * @param game_name Name of the application, used to construct the log file path
 * and filename.
 * @return true on success, false if the initialization failed.
 */
bool br_logger_init(const char *game_name);

/**
 * @brief Shuts down the logger and releases any associated resources.
 *
 * Call this at application shutdown.
 */
void br_logger_shutdown(void);

#endif // BR_LOGGER_H
