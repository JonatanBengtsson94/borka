#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PI 3.141592

#define PADDLE_SPEED 220
#define BALL_SPEED 60
// Seconds each ball squash frame shows after a bounce.
#define BALL_SQUASH_FRAME_TIME 0.04f
// Ball trail segments. Each shows where the ball was TRAIL_STEP_TIME seconds
// before the previous one, so the trail stretches as the ball speeds up.
#define TRAIL_LENGTH 3
#define TRAIL_STEP_TIME 0.03f
// Pixels the ball travels between recorded path points.
#define TRAIL_POINT_SPACING 2.0f
#define GAME_WIDTH 320
#define GAME_HEIGHT 180

#define MAX_FPS 500

// Screen shake when a brick breaks: how long it lasts, how far the world
// moves at the start (fading to nothing), and how often the offset changes.
#define SHAKE_DURATION 0.15f
#define SHAKE_STRENGTH 2
#define SHAKE_STEP_TIME (1.0f / 60.0f)

// Seconds a menu ignores key presses after it appears, so keys still being
// mashed from the last run do not start a new one straight away.
#define MENU_INPUT_DELAY 1.0

#endif // CONSTANTS_H
