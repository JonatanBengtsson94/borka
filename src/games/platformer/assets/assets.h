#ifndef ASSETS_H
#define ASSETS_H

#include "borka.h"

typedef struct {
  BrTexture *player_texture;
} Textures;

typedef struct {
  Textures textures;
} Assets;

// Expects a zeroed Assets, so a partial load can be cleaned up.
bool assets_load(Assets *assets);
void assets_destroy(Assets *assets);

#endif // ASSETS_H
