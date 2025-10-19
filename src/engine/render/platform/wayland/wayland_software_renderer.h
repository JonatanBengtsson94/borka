#ifndef WAYLAND_SOFTWARE_RENDERER_H
#define WAYLAND_SOFTWARE_RENDERER_H

#include "render/render.h"

BrRenderer *wayland_software_renderer_create(BrWindow *window);
void wayland_software_renderer_destroy(BrRenderer *renderer);
void wayland_software_renderer_present(BrRenderer *renderer);

#endif // WAYLAND_SOFTWARE_RENDERER_H
