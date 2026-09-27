#ifndef BORKA_DATA_STRUCTURE_H
#define BORKA_DATA_STRUCTURE_H

#include <stdbool.h>
#include <stddef.h>

// --- DYNAMIC ARRAY ---

/**
 * @brief A generic resizeable array.
 */
typedef struct {
  void *data;      /**< Pointer to the contiguous block of memory storing array
                      elements. */
  size_t length;   /**< Number of elements currently stored in the array. */
  size_t capacity; /**< The maximum numver of elements that can be stored before
                      reallocation. */
  size_t element_size; /**< Size of each individual element in bytes. */
} BrDynamicArray;

/**
 * @brief Initializes a dynamic array. Allocating memory for its data.
 *
 * @param array The array that should be initialized. Must not be NULL.
 * @param element_size Size of each element in bytes. Must be greater than 0.
 * @param initial_length The initial number of elements the array can store
 * before reallocation. Must be greater than 0.
 * @return True on success, false if memory allocation fails.
 *
 * @note Should call br_dynamic_array_free() when array no longer needed.
 */
bool br_dynamic_array_init(BrDynamicArray *array, size_t element_size,
                           size_t initial_length);

/**
 * @brief Cleans up the arrays allocated data.
 *
 * @param array The array that should have its data freed. Passing NULL is
 * safe and does nothing.
 */
void br_dynamic_array_free(BrDynamicArray *array);

/**
 * @brief Adds an element to the dynamic array.
 *
 * @param array The array to add an element to. Must not be NULL.
 * @param element The element that should be added. Must not be NULL.
 * @return True on success, false if growing the array fails.
 */
bool br_dynamic_array_add(BrDynamicArray *array, const void *element);

/**
 * @brief Removes an element from the dynamic array.
 *
 * Swaps the last element into the removed slot, so element order is not
 * kept.
 *
 * @param array The array to remove an element from. Must not be NULL.
 * @param index Index of the element that should be removed. Must be less
 * than the array's length.
 * @return Always true.
 */
bool br_dynamic_array_remove(BrDynamicArray *array, size_t index);

#endif // BORKA_DATA_STRUCTURE_H
