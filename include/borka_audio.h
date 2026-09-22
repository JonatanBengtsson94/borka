#ifndef BORKA_AUDIO_H
#define BORKA_AUDIO_H

#include <stdint.h>

/**
 * Audio format the engine plays back. Sounds are handed to the backend
 * as-is, so every loaded file has to already match these.
 */
#define BR_AUDIO_SAMPLE_RATE 22050
#define BR_AUDIO_CHANNELS 1
#define BR_AUDIO_BITS_PER_SAMPLE 8

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
 * @note Which file formats are recognised depends on the AUDIO_FORMATS
 *       build option (wav, flac).
 * @note Audio must match BR_AUDIO_SAMPLE_RATE, BR_AUDIO_CHANNELS and
 *       BR_AUDIO_BITS_PER_SAMPLE.
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
