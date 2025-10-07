#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_FATAL,
} LogLevel;

void logger_init(const char *filename);
void logger_shutdown();
void logger_message(LogLevel level, const char *message);

#endif
