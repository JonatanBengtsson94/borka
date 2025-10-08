#include "logger.h"
#include <stdio.h>
#include <time.h>

static FILE *log_file = NULL;

static const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR",
                                      "FATAL"};

void logger_init(const char *filename) {
  log_file = fopen(filename, "w");
  if (!log_file) {
    fprintf(stderr, "Failed to open log file: %s\n", filename);
  }
}

void logger_shutdown(void) {
  if (log_file) {
    fclose(log_file);
    log_file = NULL;
  }
}

void logger_message(LogLevel level, const char *message) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char time_buf[9];
  strftime(time_buf, sizeof(time_buf), "%H:%M:%S", t);

  fprintf(stdout, "[%s] [%s] %s\n", time_buf, level_strings[level], message);

  if (log_file) {
    fprintf(stdout, "[%s] [%s] %s\n", time_buf, level_strings[level], message);
    fflush(log_file);
  }
}
