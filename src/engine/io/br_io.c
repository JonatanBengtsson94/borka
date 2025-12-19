#include "pch.h"

#include "borka_log.h"
#include "br_io.h"

uint8_t *read_entire_file(const char *filepath, size_t *out_size) {
  FILE *fp = fopen(filepath, "rb");
  if (!fp) {
    BR_LOG_ERROR("Could not open file '%s'", filepath);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  long pos = ftell(fp);
  if (pos < 0) {
    BR_LOG_ERROR("ftell failed on '%s'", filepath);
    fclose(fp);
    return NULL;
  }
  size_t file_size = (size_t)pos;
  fseek(fp, 0, SEEK_SET);

  uint8_t *data = malloc(file_size);
  if (!data) {
    BR_LOG_ERROR("Failed to allocate data for '%s'", filepath);
    fclose(fp);
    return NULL;
  }

  size_t bytes_read = fread(data, 1, file_size, fp);
  fclose(fp);

  if (bytes_read != file_size) {
    BR_LOG_ERROR("Failed to read entire file: '%s'", filepath);
    free(data);
    return NULL;
  }

  *out_size = file_size;
  return data;
}
