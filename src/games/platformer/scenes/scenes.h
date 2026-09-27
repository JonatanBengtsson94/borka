#ifndef SCENES_H
#define SCENES_H

#include "game.h"

typedef enum {
  SCENE_NONE,
  SCENE_SANDBOX,
} SceneId;

bool scene_load(Game *game, SceneId scene);

#endif // SCENES_H
