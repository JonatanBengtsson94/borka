#include "engine.h"

int main() {
  if (init() != 0) {
    return -1;
  }

  shutdown();

  return 0;
}
