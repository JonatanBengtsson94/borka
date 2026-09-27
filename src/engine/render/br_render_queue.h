#ifndef BR_RENDER_QUEUE_H
#define BR_RENDER_QUEUE_H

#include "borka_font.h"
#include "borka_math.h"
#include "borka_texture.h"
#include <stdbool.h>
#include <stdint.h>

// Text takes one command per character, so this leaves plenty of room for a
// screen of text on top of the world.
#define BR_RENDER_QUEUE_CAPACITY 4096
// Distinct layers that can have an offset at once.
#define BR_RENDER_QUEUE_MAX_LAYER_OFFSETS 16

typedef enum {
  BR_DRAW_RECTANGLE_FILLED,
  BR_DRAW_RECTANGLE_OUTLINED,
  BR_DRAW_TEXTURE,
  BR_DRAW_TEXTURE_REGION,
} BrDrawType;

typedef struct {
  int layer;
  uint32_t sequence; // Submission order, breaks ties within a layer.
  BrDrawType type;
  BrVec2 position;
  union {
    struct {
      BrVec2 size;
      int color;
    } rectangle;
    const BrTexture *texture;
    BrTextureRegion region;
  };
} BrDrawCommand;

typedef struct {
  int layer;
  BrVec2 offset;
} BrLayerOffset;

// Draws recorded during a frame, drawn in order at present. Backends own one
// and run its commands; recording and ordering are shared.
typedef struct {
  BrDrawCommand commands[BR_RENDER_QUEUE_CAPACITY];
  int count;
  // Kept across frames: a reset only discards draws.
  BrLayerOffset layer_offsets[BR_RENDER_QUEUE_MAX_LAYER_OFFSETS];
  int layer_offset_count;
  int clear_color;
  bool clear_requested;
  bool overflowed; // Already warned about dropped draws this frame.
} BrRenderQueue;

// Discards every recorded draw, ready for the next frame.
void br_render_queue_reset(BrRenderQueue *queue);

// Records a draw and returns it for the caller to fill in, or NULL if the
// queue is full and the draw is dropped.
BrDrawCommand *br_render_queue_push(BrRenderQueue *queue, int layer,
                                    BrDrawType type, BrVec2 position);

// Records text as one texture region draw per glyph. The text and font are
// only read during this call. If the whole text does not fit, none of it is
// recorded.
void br_render_queue_push_text(BrRenderQueue *queue, int layer,
                               const BrFont *font, const char *text,
                               BrVec2 position);

// Orders commands by layer, then submission order.
void br_render_queue_sort(BrRenderQueue *queue);

// Sets the offset added to every draw on a layer at present. Returns false
// if every offset slot is taken by other layers.
bool br_render_queue_set_layer_offset(BrRenderQueue *queue, int layer,
                                      BrVec2 offset);

// The offset for a layer, or {0, 0} if none was set.
BrVec2 br_render_queue_layer_offset(const BrRenderQueue *queue, int layer);

#endif // BR_RENDER_QUEUE_H
