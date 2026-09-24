#ifndef SCENE_H
#define SCENE_H

#include "borka.h"

typedef enum {
  SCENE_NONE, // Zero value, so a zeroed GameState starts with no scene.
  SCENE_START,
  SCENE_LEVEL_01,
  SCENE_GAME_OVER,
} SceneId;

// What a scene shows and plays beyond its entities. Assets are borrowed from
// GameState, which owns them, so scenes can share textures and tracks.
typedef struct {
  SceneId id;
  const BrTexture *background; // NULL leaves just the clear color.
  BrSound *music;              // Looped while the scene is active, or NULL.
  double elapsed;              // Seconds since the scene was loaded.
  bool input_ready; // Menu prompt is shown and a key press starts a run.
} Scene;

#endif // SCENE_H
