#include "pch.h"

#include "borka_log.h"
#include "renderer/br_render_queue.h"

static void warn_overflow(BrRenderQueue *queue, const char *what) {
  if (queue->overflowed)
    return;
  queue->overflowed = true;
  BR_LOG_WARN("Render queue %s is full, dropping draws for this frame", what);
}

void br_render_queue_reset(BrRenderQueue *queue) {
  assert(queue);
  queue->count = 0;
  queue->clear_requested = false;
  queue->overflowed = false;
}

BrDrawCommand *br_render_queue_push(BrRenderQueue *queue, int layer,
                                    BrDrawType type, BrVec2 position) {
  assert(queue);

  if (queue->count >= BR_RENDER_QUEUE_CAPACITY) {
    warn_overflow(queue, "command buffer");
    return NULL;
  }

  BrDrawCommand *command = &queue->commands[queue->count];
  command->layer = layer;
  command->sequence = queue->count;
  command->type = type;
  command->position = position;
  queue->count++;
  return command;
}

void br_render_queue_push_text(BrRenderQueue *queue, int layer,
                               const BrFont *font, const char *text,
                               BrVec2 position) {
  assert(queue && font && text && font->font_atlas);

  // Check up front, so a full queue drops the whole text rather than
  // drawing the start of it.
  int glyphs = 0;
  for (int i = 0; text[i]; i++)
    if (font->glyph_positions[(unsigned char)text[i]].x >= 0)
      glyphs++;
  if (queue->count + glyphs > BR_RENDER_QUEUE_CAPACITY) {
    warn_overflow(queue, "command buffer");
    return;
  }

  BrVec2 cursor = position;
  for (int i = 0; text[i]; i++) {
    BrVec2 glyph = font->glyph_positions[(unsigned char)text[i]];
    // Characters the font has no glyph for are left blank.
    if (glyph.x >= 0) {
      BrDrawCommand *command =
          br_render_queue_push(queue, layer, BR_DRAW_TEXTURE_REGION, cursor);
      command->region = (BrTextureRegion){
          .texture = font->font_atlas, .position = glyph,
          .size = font->glyph_size};
    }

    // Every character, blank or not, takes the same space (monospace).
    cursor.x += font->glyph_size.x + font->spacing.x;
  }
  BR_LOG_TRACE("Queued text \"%s\" as %d glyphs on layer %d", text, glyphs,
               layer);
}

static int compare_commands(const void *a, const void *b) {
  const BrDrawCommand *ca = a;
  const BrDrawCommand *cb = b;
  if (ca->layer != cb->layer)
    return (ca->layer > cb->layer) - (ca->layer < cb->layer);
  // Sequences are unique, so the order is fully determined.
  return (ca->sequence > cb->sequence) - (ca->sequence < cb->sequence);
}

void br_render_queue_sort(BrRenderQueue *queue) {
  assert(queue);
  qsort(queue->commands, queue->count, sizeof(BrDrawCommand),
        compare_commands);
}
