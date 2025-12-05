#include "borka_data_structure.h"
#include "borka_log.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static bool br_dynamic_array_grow(BrDynamicArray *array) {
  size_t new_capacity = array->capacity * 2;

  void *new_data = realloc(array->data, new_capacity * array->element_size);
  if (!new_data) {
    BR_LOG_ERROR("Failed to grow array");
    return false;
  }

  array->data = new_data;
  array->capacity = new_capacity;
  return true;
}

bool br_dynamic_array_init(BrDynamicArray *array, size_t element_size,
                           size_t initial_length) {
  if (element_size == 0 || initial_length == 0) {
    BR_LOG_ERROR("Failed to init dynamic array, invalid arguments");
    return false;
  }

  array->length = 0;
  array->capacity = initial_length;
  array->element_size = element_size;

  array->data = malloc(array->capacity * array->element_size);
  if (!array->data) {
    BR_LOG_ERROR("Failed to allocate array data");
    return false;
  }

  return true;
}

void br_dynamic_array_free(BrDynamicArray *array) {
  if (array && array->data) {
    free(array->data);
  }
}

bool br_dynamic_array_add(BrDynamicArray *array, const void *element) {
  assert(array);
  assert(array->data);
  assert(array->capacity > 0);

  if (array->length == array->capacity) {
    if (!br_dynamic_array_grow(array)) {
      BR_LOG_ERROR("Failed to add element to array");
      return false;
    }
  }

  void *destination =
      (char *)array->data + (array->length * array->element_size);

  memcpy(destination, element, array->element_size);
  array->length++;

  return true;
}

bool br_dynamic_array_remove(BrDynamicArray *array, size_t index) {
  assert(array);
  assert(array->data);
  assert(array->capacity > 0);
  assert(index > 0);
  assert(index >= array->length);

  size_t last_index = array->length - 1;

  if (index != last_index) {
    void *element_at_index =
        (char *)array->data + (index * array->element_size);
    void *last_element =
        (char *)array->data + (last_index * array->element_size);

    memcpy(element_at_index, last_element, array->element_size);
  }
  array->length--;

  return true;
}
