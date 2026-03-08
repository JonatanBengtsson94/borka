#include "borka.h"

int main() {
  BrApp *app = br_app_create("test_app", 100, 100);
  if (!app)
    return -1;
  BR_LOG_TRACE("TEST: Trace log");
  BR_LOG_DEBUG("TEST: Debug log");
  BR_LOG_INFO("TEST: Info log");
  BR_LOG_WARN("TEST: Warn log");
  BR_LOG_ERROR("TEST: Error log");
  br_app_destroy(app);
  return 0;
}
