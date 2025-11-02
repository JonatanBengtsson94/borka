#ifndef BR_SOFTWARE_RENDERER_H
#define BR_SOFTWARE_RENDERER_H

#include "math/br_math.h"

void software_clear(int *pixels, int width, int height, int color);
void software_draw_triangle(int *pixels, int width, int height, BrVec2 v0,
                            BrVec2 v1, BrVec2 v2, int color);

#endif
