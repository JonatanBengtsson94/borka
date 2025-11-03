#ifndef BORKA_H
#define BORKA_H

#include "renderer/br_renderer.h"
#include "window/br_window.h"

typedef struct BrApp {
  BrWindow *window;
  BrRenderer *renderer;
} BrApp;

BrApp *br_app_create(const char *title, int width, int height);
void br_app_destroy(BrApp *app);

#endif // BORKA_H
