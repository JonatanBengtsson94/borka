#include "engine.h"
#include "logger.h"

int engine_init(const char *game_name) {
  logger_init(game_name);
  LOG_INFO("Engine initialized");
  return 0;
}

int engine_shutdown(void) {
  LOG_INFO("Engine shutdown");
  logger_shutdown();
  return 0;
}
