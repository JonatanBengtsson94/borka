#include "borka.h"
#include "borka_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

BrTexture *br_texture_load(const char *filepath) {
  BR_LOG_DEBUG("Loading texture");
  int width, height;
  unsigned char *data = stbi_load(filepath, &width, &height, NULL, 0);
  if (!data) {
    BR_LOG_ERROR("Failed to load image: %s", stbi_failure_reason());
    return NULL;
  }

  BrTexture *texture = malloc(sizeof(BrTexture));
  if (!texture) {
    BR_LOG_ERROR("Failed to allocate texture");
    stbi_image_free(data);
    return NULL;
  }

  texture->width = width;
  texture->height = height;
  texture->pixels = malloc((width * height * sizeof(int)));
  if (!texture->pixels) {
    BR_LOG_ERROR("Failed to allocate pixels");
    stbi_image_free(data);
    free(texture);
    return NULL;
  }

  // Convert RGBA bytes to ARGB int (0xAARRGGBB)
  for (int i = 0; i < width * height; ++i) {
    unsigned char r = data[i * 4 + 0];
    unsigned char g = data[i * 4 + 1];
    unsigned char b = data[i * 4 + 2];
    unsigned char a = data[i * 4 + 3];
    texture->pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }

  stbi_image_free(data); // free original byte data

  return texture;
}

void br_texture_destroy(BrTexture *texture) {
  if (texture) {
    if (texture->pixels) {
      free(texture->pixels);
    }
    free(texture);
  }
}
