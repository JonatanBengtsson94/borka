#ifndef BORKA_LOG_H
#define BORKA_LOG_H

#define BR_LL_TRACE 0
#define BR_LL_DEBUG 1
#define BR_LL_INFO 2
#define BR_LL_WARN 3
#define BR_LL_ERROR 4
#define BR_LL_FATAL 5

/**
 * @brief Describes the severity of a log message.
 */
typedef enum {
  BR_LOG_LEVEL_TRACE = BR_LL_TRACE, /**< Trace-level messages, useful for
                         step-by-step execution analysis. */
  BR_LOG_LEVEL_DEBUG =
      BR_LL_DEBUG, /**< Debug-level messages, useful for development. */
  BR_LOG_LEVEL_INFO =
      BR_LL_INFO, /**< Informational messages, general runtime info. */
  BR_LOG_LEVEL_WARN = BR_LL_WARN, /**< Warning messages, potential issues. */
  BR_LOG_LEVEL_ERROR =
      BR_LL_ERROR, /**< Error messages, failures that allow continuing. */
  BR_LOG_LEVEL_FATAL =
      BR_LL_FATAL, /**< Fatal errors, likely leading to program termination.
                    */
} BrLogLevel;

/**
 * @brief Sends a message to the logger at the given log level.
 *
 * Should not be used directly — prefer the LOG_* macros.
 *
 * @param level The severity level of the message.
 * @param format The format string (printf-style).
 * @param ... Variable arguments for the format string.
 */
void _br_logger_message(BrLogLevel level, const char *format, ...);

/**
 * @def BR_LOG_TRACE(format, ...)
 * @brief Logs a trace-level message (if LOG_LEVEL_MIN <= TRACE).
 */
#if BR_LL_TRACE >= BR_LOG_LEVEL_MIN
#define BR_LOG_TRACE(...) _br_logger_message(BR_LOG_LEVEL_TRACE, __VA_ARGS__)
#else
#define BR_LOG_TRACE(...) ((void)0)
#endif

/**
 * @def BR_LOG_DEBUG(format, ...)
 * @brief Logs a debug-level message (if LOG_LEVEL_MIN <= DEBUG).
 */
#if BR_LL_DEBUG >= BR_LOG_LEVEL_MIN
#define BR_LOG_DEBUG(...) _br_logger_message(BR_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define BR_LOG_DEBUG(...) ((void)0)
#endif

/**
 * @def BR_LOG_INFO(format, ...)
 * @brief Logs an info-level message (if LOG_LEVEL_MIN <= INFO).
 */
#if BR_LL_DEBUG >= BR_LOG_LEVEL_MIN
#define BR_LOG_INFO(...) _br_logger_message(BR_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define BR_LOG_INFO(...) ((void)0)
#endif

/**
 * @def BR_LOG_WARN(format, ...)
 * @brief Logs a warning-level message (if LOG_LEVEL_MIN <= WARN).
 */
#if BR_LL_WARN >= BR_LOG_LEVEL_MIN
#define BR_LOG_WARN(...) _br_logger_message(BR_LOG_LEVEL_WARN, __VA_ARGS__)
#else
#define BR_LOG_WARN(...) ((void)0)
#endif

/**
 * @def BR_LOG_ERROR(format, ...)
 * @brief Logs an error-level message (if LOG_LEVEL_MIN <= ERROR).
 */
#if BR_LL_ERROR >= BR_LOG_LEVEL_MIN
#define BR_LOG_ERROR(...) _br_logger_message(BR_LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define BR_LOG_ERROR(...) ((void)0)
#endif

/**
 * @def BR_LOG_FATAL(format, ...)
 * @brief Logs a fatal-level message (if LOG_LEVEL_MIN <= FATAL).
 */
#if BR_LL_FATAL >= BR_LOG_LEVEL_MIN
#define BR_LOG_FATAL(...) _br_logger_message(BR_LOG_LEVEL_FATAL, __VA_ARGS__)
#else
#define BR_LOG_FATAL(...) ((void)0)
#endif

#endif // BORKA_LOG
