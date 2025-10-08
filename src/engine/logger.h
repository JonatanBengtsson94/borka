#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_FATAL,
} LogLevel;

#ifndef LOG_LEVEL_MIN
#define LOG_LEVEL_MIN LOG_LEVEL_DEBUG
#endif

void logger_init(const char *filename);
void logger_shutdown(void);
void logger_message(LogLevel level, const char *message);

// Compile-time log level filtering macros
#if LOG_LEVEL_DEBUG >= LOG_LEVEL_MIN
#define LOG_DEBUG(msg) logger_message(LOG_LEVEL_DEBUG, msg)
#else
#define LOG_DEBUG(msg) ((void)0)
#endif

#if LOG_LEVEL_INFO >= LOG_LEVEL_MIN
#define LOG_INFO(msg) logger_message(LOG_LEVEL_INFO, msg)
#else
#define LOG_INFO(msg) ((void)0)
#endif

#if LOG_LEVEL_WARN >= LOG_LEVEL_MIN
#define LOG_WARN(msg) logger_message(LOG_LEVEL_WARN, msg)
#else
#define LOG_WARN(msg) ((void)0)
#endif

#if LOG_LEVEL_ERROR >= LOG_LEVEL_MIN
#define LOG_ERROR(msg) logger_message(LOG_LEVEL_ERROR, msg)
#else
#define LOG_ERROR(msg) ((void)0)
#endif

#if LOG_LEVEL_FATAL >= LOG_LEVEL_MIN
#define LOG_FATAL(msg) logger_message(LOG_LEVEL_FATAL, msg)
#else
#define LOG_FATAL(msg) ((void)0)
#endif

#endif
