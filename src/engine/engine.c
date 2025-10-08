#include "engine.h"
#include "logger.h"

int engine_init(void) {
  logger_message(LOG_LEVEL_INFO, "Engine initialized");
  return 0;
}

int engine_shutdown(void) {
  logger_message(LOG_LEVEL_INFO, "Engine shutdown");
  return 0;
}
