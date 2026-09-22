#ifndef BR_FLAC_H
#define BR_FLAC_H

#include "borka_audio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Reports whether the given bytes look like a FLAC file.
 *
 * Only inspects the signature, so a true result means the data is worth
 * handing to br_flac_load(), not that it decodes successfully.
 *
 * @param data Raw bytes of the file.
 * @param size Number of bytes in data.
 * @return true if the data carries a FLAC signature.
 */
bool br_flac_can_load(const uint8_t *data, size_t size);

/**
 * @brief Decodes a FLAC file already loaded into memory into a BrSound.
 *
 * @param data Raw bytes of the FLAC file, starting at the "fLaC" signature.
 * @param size Number of bytes in data.
 * @return The newly created BrSound instance, or NULL on failure.
 *
 * @note Only supports 8bit mono FLAC streams.
 */
BrSound *br_flac_load(const uint8_t *data, size_t size);

#endif // BR_FLAC_H
