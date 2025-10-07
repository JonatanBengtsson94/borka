#include "engine.h"
#include "logger.h"

int engine_init() {
  logger_message(LOG_LEVEL_INFO, "Engine initialized");
  return 0;
}

int engine_shutdown() {
  logger_message(LOG_LEVEL_INFO, "Engine shutdown");
  return 0;
}
