#ifndef LOGGER_H
#define LOGGER_H

/**
 * @file logger.h
 * @brief Simple logging system with compile-time log level filtering.
 *
 * This logger provides a minimal interface for structured log messages.
 * It supports multiple log levels and allows compile-time filtering to
 * strip out unwanted verbosity in production builds.
 */

/**
 * @enum LogLevel
 * @brief Describes the severity of a log message.
 */
typedef enum {
  LOG_LEVEL_DEBUG, /**< Debug-level messages, useful for development. */
  LOG_LEVEL_INFO,  /**< Informational messages, general runtime info. */
  LOG_LEVEL_WARN,  /**< Warning messages, potential issues. */
  LOG_LEVEL_ERROR, /**< Error messages, failures that allow continuing. */
  LOG_LEVEL_FATAL, /**< Fatal errors, likely leading to program termination. */
} LogLevel;

/**
 * @def LOG_LEVEL_MIN
 * @brief Minimum log level to include at compile time.
 *
 * Log macros below this level will be compiled out.
 * Define this macro before including the logger to override.
 */
#ifndef LOG_LEVEL_MIN
#define LOG_LEVEL_MIN LOG_LEVEL_DEBUG
#endif

/**
 * @brief Initializes the logger system.
 *
 * Should be called once at application startup before any log messages.
 *
 * @param game_name Name of the application, used to construct the log file path
 * and filename.
 */
void logger_init(const char *game_name);

/**
 * @brief Shuts down the logger and releases any associated resources.
 *
 * Call this at application shutdown.
 */
void logger_shutdown(void);

/**
 * @brief Sends a message to the logger at the given log level.
 *
 * Typically not used directly — prefer the LOG_* macros.
 *
 * @param level The severity level of the message.
 * @param message The message to log (null-terminated string).
 */
void logger_message(LogLevel level, const char *message);

/**
 * @def LOG_DEBUG(msg)
 * @brief Logs a debug-level message (if LOG_LEVEL_MIN <= DEBUG).
 */
#if LOG_LEVEL_DEBUG >= LOG_LEVEL_MIN
#define LOG_DEBUG(msg) logger_message(LOG_LEVEL_DEBUG, msg)
#else
#define LOG_DEBUG(msg) ((void)0)
#endif

/**
 * @def LOG_INFO(msg)
 * @brief Logs an info-level message (if LOG_LEVEL_MIN <= INFO).
 */
#if LOG_LEVEL_INFO >= LOG_LEVEL_MIN
#define LOG_INFO(msg) logger_message(LOG_LEVEL_INFO, msg)
#else
#define LOG_INFO(msg) ((void)0)
#endif

/**
 * @def LOG_WARN(msg)
 * @brief Logs a warning-level message (if LOG_LEVEL_MIN <= WARN).
 */
#if LOG_LEVEL_WARN >= LOG_LEVEL_MIN
#define LOG_WARN(msg) logger_message(LOG_LEVEL_WARN, msg)
#else
#define LOG_WARN(msg) ((void)0)
#endif

/**
 * @def LOG_ERROR(msg)
 * @brief Logs an error-level message (if LOG_LEVEL_MIN <= ERROR).
 */
#if LOG_LEVEL_ERROR >= LOG_LEVEL_MIN
#define LOG_ERROR(msg) logger_message(LOG_LEVEL_ERROR, msg)
#else
#define LOG_ERROR(msg) ((void)0)
#endif

/**
 * @def LOG_FATAL(msg)
 * @brief Logs a fatal-level message (if LOG_LEVEL_MIN <= FATAL).
 */
#if LOG_LEVEL_FATAL >= LOG_LEVEL_MIN
#define LOG_FATAL(msg) logger_message(LOG_LEVEL_FATAL, msg)
#else
#define LOG_FATAL(msg) ((void)0)
#endif

#endif // LOGGER_H
