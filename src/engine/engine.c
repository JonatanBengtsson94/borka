#include "engine.h"
#include "logger.h"

int engine_init(void) {
  logger_init("");
  LOG_INFO("Engine initialized");
  return 0;
}

int engine_shutdown(void) {
  LOG_INFO("Engine shutdown");
  logger_shutdown();
  return 0;
}
