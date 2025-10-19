#ifndef RENDER_H
#define RENDER_H

#include "window/window.h"

typedef struct BrRenderer BrRenderer;

BrRenderer *br_render_create(BrWindow *window);
void br_render_resize(int width, int height);
void br_render_draw(BrRenderer *renderer);

#endif // RENDER_H
