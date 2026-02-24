#ifndef BR_SOFTWARE_RENDERER_H
#define BR_SOFTWARE_RENDERER_H

#include "borka_font.h"
#include "borka_math.h"
#include "borka_texture.h"

void software_clear(int *pixels, BrVec2 dimensions, int color);
void software_draw_rectangle_filled(int *pixels, BrVec2 canvas_dimensions,
                                    BrVec2 posision, BrVec2 size, int color);
void software_draw_rectangle_outlined(int *pixels, BrVec2 canvas_dimensions,
                                      BrVec2 position, BrVec2 size, int color);
void software_draw_texture(int *pixels, BrVec2 canvas_dimensions,
                           BrVec2 position, const BrTexture *texture);
void software_draw_texture_region(int *pixels, BrVec2 canvas_dimensions,
                                  BrVec2 position, BrTextureRegion region);
void software_draw_text(int *pixels, BrVec2 canvas_dimensions, BrVec2 position,
                        const BrFont *font, const char *text);

#endif
