#ifndef BORKA_LOG_H
#define BORKA_LOG_H

/**
 * @brief Describes the severity of a log message.
 */
typedef enum {
  BR_LOG_LEVEL_DEBUG, /**< Debug-level messages, useful for development. */
  BR_LOG_LEVEL_INFO,  /**< Informational messages, general runtime info. */
  BR_LOG_LEVEL_WARN,  /**< Warning messages, potential issues. */
  BR_LOG_LEVEL_ERROR, /**< Error messages, failures that allow continuing. */
  BR_LOG_LEVEL_FATAL, /**< Fatal errors, likely leading to program termination.
                       */
} BrLogLevel;

/**
 * @def LOG_LEVEL_MIN
 * @brief Minimum log level to include at compile time.
 *
 * Log macros below this level will be compiled out.
 * Define this macro before including the logger to override.
 */
#ifndef BR_LOG_LEVEL_MIN
#define BR_LOG_LEVEL_MIN BR_LOG_LEVEL_DEBUG
#endif

/**
 * @brief Sends a message to the logger at the given log level.
 *
 * Should not be used directly — prefer the LOG_* macros.
 *
 * @param level The severity level of the message.
 * @param message The message to log (null-terminated string).
 */
void _br_logger_message(BrLogLevel level, const char *message);

/**
 * @def BR_LOG_DEBUG(msg)
 * @brief Logs a debug-level message (if LOG_LEVEL_MIN <= DEBUG).
 */
#if BR_LOG_LEVEL_DEBUG >= BR_LOG_LEVEL_MIN
#define BR_LOG_DEBUG(msg) _br_logger_message(BR_LOG_LEVEL_DEBUG, msg)
#else
#define BR_LOG_DEBUG(msg) ((void)0)
#endif

/**
 * @def BR_LOG_INFO(msg)
 * @brief Logs an info-level message (if LOG_LEVEL_MIN <= INFO).
 */
#if BR_LOG_LEVEL_INFO >= BR_LOG_LEVEL_MIN
#define BR_LOG_INFO(msg) _br_logger_message(BR_LOG_LEVEL_INFO, msg)
#else
#define BR_LOG_INFO(msg) ((void)0)
#endif

/**
 * @def BR_LOG_WARN(msg)
 * @brief Logs a warning-level message (if LOG_LEVEL_MIN <= WARN).
 */
#if BR_LOG_LEVEL_WARN >= BR_LOG_LEVEL_MIN
#define BR_LOG_WARN(msg) _br_logger_message(BR_LOG_LEVEL_WARN, msg)
#else
#define BR_LOG_WARN(msg) ((void)0)
#endif

/**
 * @def BR_LOG_ERROR(msg)
 * @brief Logs an error-level message (if LOG_LEVEL_MIN <= ERROR).
 */
#if BR_LOG_LEVEL_ERROR >= BR_LOG_LEVEL_MIN
#define BR_LOG_ERROR(msg) _br_logger_message(BR_LOG_LEVEL_ERROR, msg)
#else
#define BR_LOG_ERROR(msg) ((void)0)
#endif

/**
 * @def BR_LOG_FATAL(msg)
 * @brief Logs a fatal-level message (if LOG_LEVEL_MIN <= FATAL).
 */
#if BR_LOG_LEVEL_FATAL >= BR_LOG_LEVEL_MIN
#define BR_LOG_FATAL(msg) _br_logger_message(BR_LOG_LEVEL_FATAL, msg)
#else
#define BR_LOG_FATAL(msg) ((void)0)
#endif

#endif // BORKA_LOG
