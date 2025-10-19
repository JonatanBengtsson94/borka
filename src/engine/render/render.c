#include "render.h"
#include "logger.h"
#include "platform/wayland/wayland_software_renderer.h"

BrRenderer *br_render_create(BrWindow *window) {
  return wayland_software_renderer_create(window);
}

void br_render_resize(int width, int height) { BR_LOG_INFO("Window resized"); }

void br_render_draw(BrRenderer *renderer) {
  wayland_software_renderer_present(renderer);
  BR_LOG_INFO("Draw");
}
