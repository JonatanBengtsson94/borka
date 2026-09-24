#include "borka.h"

typedef enum {
  RENDERABLE_TEXTURE,
  RENDERABLE_TEXTURE_REGION,
  RENDERABLE_RECTANGLE,
  RENDERABLE_FILLED_TRIANGLE,
  RENDERABLE_TEXT,
} RenderType;

// Draw order, back to front, passed to the renderer as the draw layer.
typedef enum {
  RENDER_LAYER_BACKGROUND, // The scene's background.
  RENDER_LAYER_BEHIND,     // Effects behind the world, such as the ball trail.
  RENDER_LAYER_WORLD,  // Paddle, ball and bricks.
  RENDER_LAYER_UI,     // Text.
} RenderLayer;

typedef struct {
  RenderType type;
  RenderLayer layer;
  BrVec2 offset; // Drawn this far from the entity's position.
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

    struct {
      BrVec2 size;
      int color;
      bool filled;
    } rectangle;

    struct {
      BrVec2 v1;
      BrVec2 v2;
      BrVec2 v3;
      int colot;
    } filled_triangle;
  };
} Renderable;
