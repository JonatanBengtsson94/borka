#include "borka.h"
#include "constants.h"

// Recorded path points. The path covers at least (TRAIL_POINTS - 1) *
// TRAIL_POINT_SPACING = 30 pixels, enough for the last segment up to about
// 330 px/s. Beyond that the trail stops growing and bunches up at its end.
#define TRAIL_POINTS 16

// Afterimages following a moving entity. Its center is recorded every
// TRAIL_POINT_SPACING pixels it travels, and segment i is placed where the
// entity was (i + 1) * TRAIL_STEP_TIME seconds ago, measured along that path.
typedef struct {
  BrEntity segments[TRAIL_LENGTH];
  Position points[TRAIL_POINTS]; // Recorded centers, newest first.
  Position center_offset;        // From the entity's position to its center.
} Trail;
