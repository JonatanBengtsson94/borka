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
- **Squash on bounce** (done): the ball flattens for a couple of frames
  against whatever it bounces off. Animations choose what happens at the end
  (loop, hold or destroy), and frames can be offset to stay centred.
- **Hit flash**: the ball turns white for about 50 ms on every hit. Needs a
  white ball sprite.
- **Sparks**: a few pixels fly out from the hit point and vanish. Uses
  entities quickly, so only 3 or 4 per hit fit the budget.

## Screen

- **Screen shake** (done): the world shifts by a pixel or two for 0.15 s when
  a brick breaks. The background and text stay still.
