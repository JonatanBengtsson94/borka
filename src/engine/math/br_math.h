#ifndef BR_MATH_H
#define BR_MATH_H

/**
 * @brief Returns the smaller of two integers.
 *
 * @param a First integer to compare.
 * @param b Second integer to compare.
 * @return The smaller of `a` and `b`.
 */
static inline int min_int(int a, int b) { return a < b ? a : b; }

/**
 * @brief Returns the larger of two integers.
 *
 * @param a First integer to compare.
 * @param b Second integer to compare.
 * @return The larger of `a` and `b`.
 */
static inline int max_int(int a, int b) { return a > b ? a : b; }

/**
 * @brief 2D integer vector.
 */
typedef struct {
  int x; /**< X component */
  int y; /**< Y component */
} BrVec2;

/**
 * @brief Adds two 2D vectors.
 *
 * @param a First vector.
 * @param b Second vector.
 * @return The sum of a and b.
 */
static inline BrVec2 br_vec2_add(BrVec2 a, BrVec2 b) {
  return (BrVec2){a.x + b.x, a.y + b.y};
}

/**
 * @brief Substracts one 2D vector from another.
 *
 * @param a Minuend vector.
 * @param b Subtrahend vector.
 * @return The difference between a and b,
 */
static inline BrVec2 br_vec2_sub(BrVec2 a, BrVec2 b) {
  return (BrVec2){a.x - b.x, a.y - b.y};
}

/**
 * @brief Computes the 2D cross product of two vectors.
 *
 * @param a First vector.
 * @param b Second vector.
 * @return Scalar cross product of a and b.
 */
static inline int br_vec2_cross(BrVec2 a, BrVec2 b) {
  return a.x * b.y - a.y * b.x;
}

#endif // BR_MATH_H
