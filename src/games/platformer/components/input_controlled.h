#ifndef INPUT_CONTROLLED_H
#define INPUT_CONTROLLED_H

#include <stdbool.h>

typedef struct {
  bool left_pressed;
  bool right_pressed;
  bool jump_pressed;
} InputControlled;

#endif // INPUT_CONTROLLED_H
