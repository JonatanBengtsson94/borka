#ifndef BR_IO_H
#define BR_IO_H

#include <stddef.h>
#include <stdint.h>

uint8_t *read_entire_file(const char *filepath, size_t *out_size);

#endif // !BR_IO_H
