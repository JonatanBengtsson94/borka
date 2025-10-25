#include "borka.h"
#include "logger/logger.h"

int br_init(const char *game_name) {
  br_logger_init(game_name);
  BR_LOG_INFO("Engine initialized");
  return 0;
}

int br_shutdown(void) {
  BR_LOG_INFO("Engine shutdown");
  br_logger_shutdown();
  return 0;
}
