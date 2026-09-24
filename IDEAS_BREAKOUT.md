# Ideas

Effects and animations that could make breakout feel more polished.

## Constraints

- The renderer has no transparency or scaling. A pixel is either drawn or
  skipped, so fading or shrinking needs extra sprite frames or darker colors.
- The ECS allows 100 entities. A level uses 77 (68 bricks, ball, 3 trail
  segments, paddle and 4 walls), and brick break animations borrow from the
  remaining 23.

## Ball

- **Trail** (done): thin afterimages at the ball's recent positions, each
  smaller and darker than the last. Uses render layers to stay behind the
  ball.
- **Squash on bounce**: the ball flattens for a couple of frames on impact,
  sideways on walls and vertically on the paddle and bricks. Needs extra ball
  frames, and the animator must not destroy the ball when the animation ends.
- **Hit flash**: the ball turns white for about 50 ms on every hit. Needs a
  white ball sprite.
- **Sparks**: a few pixels fly out from the hit point and vanish. Uses
  entities quickly, so only 3 or 4 per hit fit the budget.

## Screen

- **Screen shake**: shift everything by a pixel or two for about 100 ms when
  a brick breaks. No entities or art needed.
