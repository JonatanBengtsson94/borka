#ifndef BR_SOFTWARE_RENDERER_H
#define BR_SOFTWARE_RENDERER_H

#include "borka_math.h"
#include "borka_texture.h"

void software_clear(int *pixels, int width, int height, int color);
void software_draw_filled_triangle(int *pixels, int width, int height,
                                   BrVec2 v0, BrVec2 v1, BrVec2 v2, int color);
void software_draw_rectangle_filled(int *pixels, int width, int height, int x,
                                    int y, int rect_width, int rect_height,
                                    int color);
void software_draw_rectangle_outlined(int *pixels, int width, int height, int x,
                                      int y, int rect_width, int rect_height,
                                      int color);
void software_draw_texture(int *pixels, int width, int height, int x, int y,
                           const BrTexture *texture);

#endif
