#ifndef BR_RENDERER_H
#define BR_RENDERER_H

#include "borka_render.h"
#include "borka_window.h"

BrRenderer *br_renderer_create(BrWindow *window);
void br_renderer_destroy(BrRenderer *renderer);

#endif // BR_RENDERER_H
