#ifndef BR_SOFTWARE_RENDERER_H
#define BR_SOFTWARE_RENDERER_H

#include "borka_math.h"
#include "borka_texture.h"

void software_clear(int *pixels, BrVec2 dimensions, int color);
// TODO: Fix this
void software_draw_filled_triangle(int *pixels, BrVec2 canvas_dimensions,
                                   BrVec2 v0, BrVec2 v1, BrVec2 v2, int color);
void software_draw_rectangle_filled(int *pixels, BrVec2 canvas_dimensions,
                                    BrVec2 posision, BrVec2 size, int color);
void software_draw_rectangle_outlined(int *pixels, BrVec2 canvas_dimensions,
                                      BrVec2 position, BrVec2 size, int color);
void software_draw_texture(int *pixels, BrVec2 canvas_dimensions,
                           BrVec2 position, const BrTexture *texture);

#endif
