#include "camera.h"
#include "constants.h"
#include <stdlib.h>

// Restarting rather than adding up keeps several breaks in a row from
// shaking harder and harder.
void camera_shake(Camera *camera) {
  assert(camera);
  camera->shake_time = SHAKE_DURATION;
  camera->shake_step_time = 0.0f;
  BR_LOG_TRACE("Screen shake started");
}

// Picks a new random offset every SHAKE_STEP_TIME, so the shake reads as
// movement rather than a blur at high frame rates. Its range fades with the
// time left.
void camera_update(Camera *camera, double delta_time) {
  assert(camera);

  if (camera->shake_time <= 0.0f) {
    camera->offset = (BrVec2){0, 0};
    return;
  }

  camera->shake_time -= delta_time;
  camera->shake_step_time -= delta_time;
  if (camera->shake_step_time > 0.0f)
    return;

  camera->shake_step_time += SHAKE_STEP_TIME;
  float fade =
      camera->shake_time > 0.0f ? camera->shake_time / SHAKE_DURATION : 0;
  int range = (int)(SHAKE_STRENGTH * fade + 0.5f);
  camera->offset = (BrVec2){range ? rand() % (2 * range + 1) - range : 0,
                            range ? rand() % (2 * range + 1) - range : 0};
}
