#include "components/components.h"
#include "constants.h"
#include "systems.h"
#include <math.h>

// Walks back from the current center through the recorded points and returns
// the point the given distance along that path. If the path is shorter, as
// right after spawning, it returns the oldest point.
static Position point_behind(Position center, const Position *points,
                             float distance) {
  Position from = center;
  for (int i = 0; i < TRAIL_POINTS; i++) {
    float dx = points[i].x - from.x;
    float dy = points[i].y - from.y;
    float length = sqrtf(dx * dx + dy * dy);
    if (length > 0 && length >= distance) {
      float t = distance / length;
      return (Position){from.x + dx * t, from.y + dy * t};
    }
    distance -= length;
    from = points[i];
  }
  return from;
}

void system_trail(BrRegistry *registry) {
  assert(registry);

  BrQuery *query = br_query_begin(registry, SYSTEM_TRAIL);
  while (br_query_next(query)) {
    Trail *trail = (Trail *)br_query_get_component(query, COMPONENT_TRAIL);
    Position *pos = (Position *)br_query_get_component(query, COMPONENT_POSITION);
    Velocity *vel = (Velocity *)br_query_get_component(query, COMPONENT_VELOCITY);
    assert(trail);
    assert(pos);
    assert(vel);

    Position center = {pos->x + trail->center_offset.x,
                       pos->y + trail->center_offset.y};

    float dx = center.x - trail->points[0].x;
    float dy = center.y - trail->points[0].y;
    if (dx * dx + dy * dy >= TRAIL_POINT_SPACING * TRAIL_POINT_SPACING) {
      for (int i = TRAIL_POINTS - 1; i > 0; i--)
        trail->points[i] = trail->points[i - 1];
      trail->points[0] = center;
    }

    float speed = sqrtf(vel->vx * vel->vx + vel->vy * vel->vy);
    for (int i = 0; i < TRAIL_LENGTH; i++) {
      BrEntity segment = trail->segments[i];
      Position *seg_pos =
          br_component_get(registry, COMPONENT_POSITION, segment);
      Renderable *seg_ren =
          br_component_get(registry, COMPONENT_RENDERABLE, segment);
      assert(seg_pos);
      assert(seg_ren);

      Position at = point_behind(center, trail->points,
                                 speed * (i + 1) * TRAIL_STEP_TIME);
      seg_pos->x = at.x - seg_ren->region.region.size.x / 2.0f;
      seg_pos->y = at.y - seg_ren->region.region.size.y / 2.0f;
    }
  }
}
