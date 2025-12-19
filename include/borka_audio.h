#ifndef BORKA_AUDIO_H
#define BORKA_AUDIO_H

#include <stdint.h>

/**
 * @brief Represents a sound.
 */
typedef struct {
  uint8_t *data; /**< Audio sample data. */
  uint32_t size; /**< Size of the audio data. */
} BrSound;

/**
 * @brief Creates a BrSound instance from an audio file.
 *
 * @param filepath Path to the audio file.
 * @return The newly created BrSound instance, or NULL on failure.
 *
 * @note Only supports wav audio file format.
 * @note Only supports 8bit mono audio with 22050 sample rate.
 * @note Should be destroyed with br_sound_destroy() when no longer needed.
 */
BrSound *br_sound_create(const char *filepath);

/**
 * @brief Destroys the sound instance and frees its memory.
 *
 * @param sound Sound instance to destroy.
 */
void br_sound_destroy(BrSound *sound);

/**
 * @brief Signal the audio thread to play a sound.
 *
 * This function is non-blocking.
 *
 * @param sound Sound to play. Must not be NULL.
 */
void br_play_sound(BrSound *sound);

#endif // BORKA_AUDIO_H
