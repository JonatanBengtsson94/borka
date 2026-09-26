#include "borka.h"

typedef enum {
  RENDERABLE_TEXTURE,
  RENDERABLE_TEXTURE_REGION,
  RENDERABLE_TEXT,
} RenderType;

// Draw order, back to front, passed to the renderer as the draw layer.
typedef enum {
  RENDER_LAYER_BACKGROUND,
  RENDER_LAYER_WORLD,
  RENDER_LAYER_FOREGROUND,
} RenderLayer;

typedef struct {
  RenderType type;
  RenderLayer layer;
  union {
    struct {
      BrTexture *texture;
    } texture;

    struct {
      BrTextureRegion region;
    } region;

    struct {
      BrFont *font;
      char *text;
    } text;
  };
} Renderable;
