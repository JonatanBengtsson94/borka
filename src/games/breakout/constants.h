#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PI 3.141592

#define PADDLE_SPEED 220
#define BALL_SPEED 60
// Ball trail segments. Each shows where the ball was TRAIL_STEP_TIME seconds
// before the previous one, so the trail stretches as the ball speeds up.
#define TRAIL_LENGTH 3
#define TRAIL_STEP_TIME 0.03f
// Pixels the ball travels between recorded path points.
#define TRAIL_POINT_SPACING 2.0f
#define GAME_WIDTH 320
#define GAME_HEIGHT 180

#define MAX_FPS 500

// Seconds a menu ignores key presses after it appears, so keys still being
// mashed from the last run do not start a new one straight away.
#define MENU_INPUT_DELAY 1.0

#endif // CONSTANTS_H
