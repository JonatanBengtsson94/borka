#include "borka.h"

// What an animation does after its last frame.
typedef enum {
  ANIMATION_END_LOOP,    // Start over from the first frame.
  ANIMATION_END_HOLD,    // Stay on the last frame until restarted.
  ANIMATION_END_DESTROY, // Destroy the entity, for one-off effects.
} AnimationEnd;

typedef struct {
  BrTextureRegion *frames;
  // Draw offset for each frame, or NULL. Keeps frames of another size than
  // the entity's collider centred on it.
  const BrVec2 *offsets;
  int number_of_frames;
  int current_frame;
  float frame_time;
  float elapsed_time;
  AnimationEnd on_end;
  bool finished; // Reached its last frame. Never set when looping.
} Animator;
