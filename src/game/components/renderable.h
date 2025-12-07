#include "borka.h"

typedef enum {
  RENDERABLE_SPRITE,
  RENDERABLE_RECTANGLE,
  RENDERABLE_FILLED_TRIANGLE,
} RenderType;

typedef struct {
  RenderType type;
  union {
    struct {
      BrTexture *texture;
    } sprite;

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
