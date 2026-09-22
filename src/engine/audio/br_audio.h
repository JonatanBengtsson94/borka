#ifndef BR_AUDIO_H
#define BR_AUDIO_H

#include "borka_audio.h"
#include <stdbool.h>

bool br_audio_init();
void br_audio_shutdown();
BrSound *br_sound_create(const char *filepath);

/**
 * @brief Stops every voice currently playing the given sound.
 *
 * Called before a sound is freed so the mixer cannot keep reading it.
 */
void br_audio_stop_sound(BrSound *sound);

#endif // BR_AUDIO_H
