#include "logger/br_logger.h"

int main() {
  if (!br_logger_init("logging")) {
    return 1;
  }

  BR_LOG_TRACE("TEST: Trace log");
  BR_LOG_DEBUG("TEST: Debug log");
  BR_LOG_INFO("TEST: Info log");
  BR_LOG_WARN("TEST: Warn log");
  BR_LOG_ERROR("TEST: Error log");

  br_logger_shutdown();
  return 0;
}
