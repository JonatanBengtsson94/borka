#include "borka.h"

typedef struct {
  BrTextureRegion *frames;
  int number_of_frames;
  int current_frame;
  float frame_time;
  float elapsed_time;
  bool loop;
  bool finished;
} Animator;
