#ifndef BR_LOGGER_H
#define BR_LOGGER_H

#include <stdbool.h>
#include <time.h>
#include "borka_log.h"

#define MAX_MESSAGE_LENGTH 512
#define MESSAGE_QUEUE_SIZE 256
#define MAX_LOG_SIZE (10 * 1024 * 1024)
#define MAX_LOG_FILES 5
#define MAX_LOG_FILE_PATH_SIZE 512

typedef struct {
  BrLogLevel level;
  char message[MAX_MESSAGE_LENGTH];
  time_t timestamp;
} LogMessage;

bool br_logger_init(const char *game_name);
void br_logger_shutdown(void);

#endif // BR_LOGGER_H
