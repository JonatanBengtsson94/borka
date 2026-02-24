#ifndef BORKA_TEXTURE_H
#define BORKA_TEXTURE_H

#include "borka_math.h"
#include <stdint.h>

/**
 * @brief Represent a 2D texture with ARGB pixel data.
 */
typedef struct {
  uint32_t *pixels; /**< ARGB pixel data (0xAARRGGBB format). */
  BrVec2 size;      /** The size of the texture in pixels. */
} BrTexture;

/**
 * @brief Represents a region of a 2D texture.
 */
typedef struct {
  BrTexture *texture; /**< Source texture. */
  BrVec2 position;    /**< Top left corner of the region. */
  BrVec2 size;        /**< size of the region. */
} BrTextureRegion;

/**
 * @brief Creates a BrTexture instances from an image file.
 *
 * @param filepath Path to the image file.
 * @return The newly created BrTexture instance, or NULL on failure.
 *
 * @note Only supports the png image format.
 * @note Only supports pngs with 8-bit depth and rbga color type.
 * @note Should be destroyed with br_texture_destroy() when no longer needed.
 */
BrTexture *br_texture_create(const char *filepath);

/**
 * @brief Destroys the texture instance and frees its memory.
 *
 * @param texture The BrTexture instance to destroy. Passing NULL is safe and
 * does nothing.
 */
void br_texture_destroy(BrTexture *texture);

#endif
