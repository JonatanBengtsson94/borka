#ifndef BR_WAV_H
#define BR_WAV_H

#include "borka_audio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Reports whether the given bytes look like a WAV file.
 *
 * Only inspects the RIFF/WAVE identifiers, so a true result means the data
 * is worth handing to br_wav_load(), not that it loads successfully.
 *
 * @param data Raw bytes of the file.
 * @param size Number of bytes in data.
 * @return true if the data is a RIFF container holding a WAVE form.
 */
bool br_wav_can_load(const uint8_t *data, size_t size);

/**
 * @brief Decodes a WAV file already loaded into memory into a BrSound.
 *
 * @param data Raw bytes of the WAV file, starting at the "RIFF" signature.
 * @param size Number of bytes in data.
 * @return The newly created BrSound instance, or NULL on failure.
 *
 * @note Only supports 8bit mono PCM WAV files.
 */
BrSound *br_wav_load(const uint8_t *data, size_t size);

#endif // BR_WAV_H
