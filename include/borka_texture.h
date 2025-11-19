#ifndef BORKA_TEXTURE_H
#define BORKA_TEXTURE_H

/**
 * @brief Represent a 2D texture with ARGB pixel data.
 */
typedef struct {
  int *pixels; /**< ARGB pixel data (0xAARRGGBB format). */
  int width;   /**< Texture width in pixels. */
  int height;  /**< Texture height in pixels. */
} BrTexture;

/**
 * @brief Loads a texture from a file.
 *
 * @param filepath Path to the image file.
 * @return The newly created BrTexture instance, or NULL on failure.
 *
 * @note Should be destroyed with br_texture_destroy() when no longer needed.
 */
BrTexture *br_texture_load(const char *filepath);

/**
 * @brief Destroys the texture instance and frees its memory.
 *
 * @param texture The BrTexture instance to destroy. Passing NULL is safe and
 * does nothing.
 */
void br_texture_destroy(BrTexture *texture);

#endif
