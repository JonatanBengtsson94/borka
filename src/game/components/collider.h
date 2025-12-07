#include "borka.h"
#include <stdint.h>

typedef enum {
  LAYER_PADDLE = 1 << 0,
  LAYER_BALL = 1 << 1,
  LAYER_BRICK = 1 << 2,
  LAYER_WALL = 1 << 3,
  LAYER_FLOOR = 1 << 4,
} Layer;

typedef struct {
  int width;
  int height;
  uint8_t layer;
  uint8_t mask;
} Collider;
