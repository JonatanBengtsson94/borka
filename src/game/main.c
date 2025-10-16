#include "borka.h"

int main() {
  if (br_init("breakout") != 0) {
    return -1;
  }

  br_shutdown();

  return 0;
}
