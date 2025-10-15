#include "engine.h"

int main() {
  if (engine_init("breakout") != 0) {
    return -1;
  }

  engine_shutdown();

  return 0;
}
