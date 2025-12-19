#ifndef BR_AUDIO_H
#define BR_AUDIO_H

#include "borka_audio.h"
#include <stdbool.h>

bool br_audio_init();
void br_audio_shutdown();
BrSound *br_sound_create(const char *filepath);

#endif // BR_AUDIO_H
