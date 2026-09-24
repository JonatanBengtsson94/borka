#ifndef CAMERA_H
#define CAMERA_H

#include "borka.h"

// How the world is viewed. The offset moves the world layers when drawn, and
// currently only comes from screen shake.
typedef struct {
  BrVec2 offset;
  float shake_time;      // Seconds of screen shake left.
  float shake_step_time; // Seconds until the shake offset changes.
} Camera;

// Starts a screen shake, or restarts one already running.
void camera_shake(Camera *camera);
void camera_update(Camera *camera, double delta_time);

#endif // CAMERA_H
