#ifndef BR_WINDOW_H
#define BR_WINDOW_H

#include "borka_window.h"
#include "pch.h"

BrWindow *br_window_create(const char *title, int width, int height);
void br_window_destroy(BrWindow *window);

#endif // BR_WINDOW_H
