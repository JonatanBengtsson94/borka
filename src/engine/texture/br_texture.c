#include "borka.h"
#include "borka_texture.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

const uint8_t PNG_SIGNATURE[8] = {0x89, 0x50, 0x4E, 0x47,
                                  0x0D, 0x0A, 0x1A, 0x0A};

typedef enum {
  CHUNK_IHDR = 0x49484452,
  CHUNK_IDAT = 0x49444154,
  CHUNK_IEND = 0x49454E44,
} ChunkType;

typedef struct {
  uint32_t length;
  ChunkType type;
  uint32_t crc;
} Chunk;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t color_type;
  uint8_t compression_type;
  uint8_t filter_method;
  uint8_t interlace_method;
} IHDR;

typedef enum {
  FILTER_NONE = 0,
  FILTER_SUB = 1,
  FILTER_UP = 2,
  FILTER_AVERAGE = 3,
  FILTER_PAETH = 4,
} FilterType;

// --- PNG DECODER ---

static uint32_t read_u32_be(const uint8_t *data) {
  return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
         ((uint32_t)data[2] << 8) | ((uint32_t)data[3]);
}

static uint8_t *read_entire_file(const char *filepath, size_t *out_size) {
  FILE *fp = fopen(filepath, "rb");
  if (!fp) {
    BR_LOG_ERROR("Failed to load texture: could not open file '%s'", filepath);
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

static bool validate_png_signature(const uint8_t *data, size_t file_size) {
  if (file_size < sizeof(PNG_SIGNATURE)) {
    BR_LOG_ERROR("File is too small for PNG signature");
    return false;
  }

  if (memcmp(data, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != 0) {
    BR_LOG_ERROR("Invalid PNG signature");
    return false;
  }

  return true;
}

static IHDR *parse_ihdr(const uint8_t *data, size_t file_size, size_t *offset) {
  size_t current_offset = *offset;

  if (current_offset + 8 > file_size) {
    BR_LOG_ERROR("PNG too small for IHDR chunk");
    return NULL;
  }

  Chunk ihdr_chunk;
  ihdr_chunk.length = read_u32_be(data + current_offset);
  current_offset += 4;
  ihdr_chunk.type = read_u32_be(data + current_offset);
  current_offset += 4;

  if (ihdr_chunk.type != CHUNK_IHDR) {
    BR_LOG_ERROR("First PNG chunk is not IHDR");
    return NULL;
  }

  if (ihdr_chunk.length != 13) {
    BR_LOG_ERROR("IHDR must be 13 bytes (got %u)", ihdr_chunk.length);
    return NULL;
  }

  if (current_offset + ihdr_chunk.length + 4 > file_size) {
    BR_LOG_ERROR("IHDR extends past end of file");
    return NULL;
  }

  IHDR *ihdr = malloc(sizeof(IHDR));
  if (!ihdr) {
    BR_LOG_ERROR("Failed to allocate IHDR");
    return NULL;
  }

  // Validate IHDR Chunk
  ihdr->width = read_u32_be(data + current_offset);
  ihdr->height = read_u32_be(data + current_offset + 4);
  ihdr->bit_depth = data[current_offset + 8];
  ihdr->color_type = data[current_offset + 9];
  ihdr->compression_type = data[current_offset + 10];
  ihdr->filter_method = data[current_offset + 11];
  ihdr->interlace_method = data[current_offset + 12];

  // Advance beyond IHDR + CRC
  current_offset += ihdr_chunk.length + 4;
  *offset = current_offset;

  if (ihdr->width == 0 || ihdr->height == 0) {
    BR_LOG_ERROR("Invalid PNG dimensions: %ux%u", ihdr->width, ihdr->height);
    return NULL;
  }

  if (ihdr->bit_depth != 8) {
    BR_LOG_ERROR("Only 8-bit PNG supported (got %u-bit)", ihdr->bit_depth);
    return NULL;
  }

  if (ihdr->color_type != 6) {
    BR_LOG_ERROR("Only RGBA PNG supported (got color_type %u)",
                 ihdr->color_type);
    return NULL;
  }

  if (ihdr->interlace_method != 0) {
    BR_LOG_ERROR("Interlaced PNG not supported");
    return NULL;
  }

  if (ihdr->compression_type != 0 || ihdr->filter_method != 0) {
    BR_LOG_ERROR("Invalid PNG format");
    return NULL;
  }

  BR_LOG_DEBUG(
      "PNG IHDR: width: %u, height: %u, bit_depth: %u, color_type: %u, "
      "compression_type: %u, filter_method: %u, interlace_method: %u",
      ihdr->width, ihdr->height, ihdr->bit_depth, ihdr->color_type,
      ihdr->compression_type, ihdr->filter_method, ihdr->interlace_method);

  return ihdr;
}

static uint8_t *collect_compressed_data(const uint8_t *data, size_t size,
                                        size_t *offset, size_t *out_size) {
  size_t current_offset = *offset;
  size_t capacity = 0;
  size_t total_bytes = 0;
  uint8_t *buffer = NULL;

  while (current_offset + 8 <= size) {
    Chunk chunk;
    chunk.length = read_u32_be(data + current_offset);
    current_offset += 4;
    chunk.type = (ChunkType)read_u32_be(data + current_offset);
    current_offset += 4;

    if (current_offset + chunk.length + 4 > size) {
      BR_LOG_ERROR("Invalid PNG, incomplete chunk");
      free(buffer);
      return NULL;
    }

    if (chunk.type == CHUNK_IEND) {
      BR_LOG_DEBUG("Found IEND chunk");
      break;
    }

    if (chunk.type == CHUNK_IDAT) {
      // Expand buffer if needed
      if (total_bytes + chunk.length > capacity) {
        capacity = total_bytes + chunk.length;
        uint8_t *new_data = realloc(buffer, capacity);
        if (!new_data) {
          BR_LOG_ERROR("Failed to allocate memory for IDAT");
          free(buffer);
          return NULL;
        }
        buffer = new_data;
      }

      memcpy(buffer + total_bytes, data + current_offset, chunk.length);
      total_bytes += chunk.length;

      BR_LOG_DEBUG("Found IDAT chunk: %u bytes (total: %zu)", chunk.length,
                   total_bytes);
    }
    current_offset += chunk.length + 4; // Data + CRC
  }

  *offset = current_offset;
  *out_size = total_bytes;
  return buffer;
}

static uint8_t *decompress_data(const uint8_t *compressed_data,
                                size_t compressed_size,
                                size_t uncompressed_size) {
  uint8_t *uncompressed_data = malloc(uncompressed_size);
  if (!uncompressed_data) {
    BR_LOG_ERROR("Failed to allocate uncompressed data");
    return NULL;
  }

  z_stream stream = {0};
  stream.next_in = (Bytef *)compressed_data;
  stream.avail_in = compressed_size;
  stream.next_out = uncompressed_data;
  stream.avail_out = uncompressed_size;

  int ret = inflateInit(&stream);
  if (ret != Z_OK) {
    BR_LOG_ERROR("Init failed");
    free(uncompressed_data);
    return NULL;
  }

  ret = inflate(&stream, Z_FINISH);
  if (ret != Z_STREAM_END) {
    BR_LOG_ERROR("inflate fail");
    inflateEnd(&stream);
    free(uncompressed_data);
    return NULL;
  }

  inflateEnd(&stream);
  BR_LOG_DEBUG("Decompressed %lu bytes", stream.total_out);

  return uncompressed_data;
}

static uint8_t *unfilter_scanlines(const uint8_t *filtered_data, uint32_t width,
                                   uint32_t height, size_t scanline_size,
                                   size_t output_size) {
  uint8_t *unfiltered_data = malloc(output_size);
  if (!unfiltered_data) {
    BR_LOG_ERROR("Failed to allocate unfiltered data");
    return NULL;
  }

  size_t bytes_per_pixel = 4; // RGBA
  size_t row_bytes = scanline_size - 1;

  for (uint32_t y = 0; y < height; y++) {
    uint8_t filter_type = filtered_data[y * scanline_size];

    int src_start = y * scanline_size + 1;
    int dst_start = y * row_bytes;

    switch (filter_type) {
    case FILTER_NONE:
      for (size_t x = 0; x < row_bytes; x++) {
        unfiltered_data[dst_start + x] = filtered_data[src_start + x];
      }
      break;

    case FILTER_SUB:
      for (size_t x = 0; x < row_bytes; x++) {
        uint8_t left = 0;
        if (x >= bytes_per_pixel) {
          left = unfiltered_data[dst_start + x - bytes_per_pixel];
        }
        unfiltered_data[dst_start + x] = filtered_data[src_start + x] + left;
      }
      break;

    case FILTER_UP:
      for (size_t x = 0; x < row_bytes; x++) {
        uint8_t above = 0;
        if (y > 0) {
          above = unfiltered_data[dst_start + x - row_bytes];
        }
        unfiltered_data[dst_start + x] = filtered_data[src_start + x] + above;
      }
      break;

    case FILTER_AVERAGE:
      BR_LOG_ERROR("Unsupported filter method average");
      break;
    case FILTER_PAETH:
      BR_LOG_ERROR("Unsupported filter method paeth");
      break;
    default:
      BR_LOG_ERROR("Unknown filter method (got %u)", filter_type);
      break;
    }
  }
  return unfiltered_data;
}

// --- PUBLIC API ---

BrTexture *br_texture_load(const char *filepath) {
  // Read file
  size_t file_size;
  uint8_t *file_data = read_entire_file(filepath, &file_size);
  if (!file_data) {
    BR_LOG_ERROR("Failed to read file: '%s'", filepath);
    return NULL;
  }

  // Read and validate signature
  if (!validate_png_signature(file_data, file_size)) {
    BR_LOG_ERROR("Could not validate png signature: '%s'", filepath);
    free(file_data);
    return NULL;
  }

  // Read and validate ihdr
  size_t offset = sizeof(PNG_SIGNATURE);
  IHDR *ihdr = parse_ihdr(file_data, file_size, &offset);
  if (!ihdr) {
    BR_LOG_ERROR("Failed to parse ihdr: '%s'", filepath);
    free(file_data);
    return NULL;
  }

  // Read idat chunks
  size_t compressed_size = 0;
  uint8_t *compressed_data =
      collect_compressed_data(file_data, file_size, &offset, &compressed_size);
  free(file_data);
  if (compressed_size == 0) {
    BR_LOG_ERROR("No IDAT chunks found: '%s'", filepath);
    free(ihdr);
    free(compressed_data);
    return NULL;
  }

  size_t bytes_per_pixel = 4;                               // RGBA
  size_t scanline_size = ihdr->width * bytes_per_pixel + 1; // +1 filter byte
  size_t uncompressed_size = scanline_size * ihdr->height;

  // Decompress data
  uint8_t *uncompressed_data =
      decompress_data(compressed_data, compressed_size, uncompressed_size);
  free(compressed_data);
  if (!uncompressed_data) {
    BR_LOG_ERROR("Failed to decompress data: '%s'", filepath);
    return NULL;
  }

  // Unfilter data
  size_t unfiltered_size = bytes_per_pixel * ihdr->width * ihdr->height;
  uint8_t *unfiltered_data =
      unfilter_scanlines(uncompressed_data, ihdr->width, ihdr->height,
                         scanline_size, unfiltered_size);
  free(uncompressed_data);
  if (!unfiltered_data) {
    BR_LOG_ERROR("Failed to unfilter data: '%s'", filepath);
    free(ihdr);
    return NULL;
  }

  // Allocate BrTexture
  BrTexture *texture = malloc(sizeof(BrTexture));
  if (!texture) {
    BR_LOG_ERROR("Failed to allocate texture: '%s'", filepath);
    free(unfiltered_data);
    free(ihdr);
    return NULL;
  }

  texture->width = ihdr->width;
  texture->height = ihdr->height;
  size_t num_pixels = ihdr->width * ihdr->height;
  free(ihdr);
  texture->pixels = malloc(num_pixels * sizeof(int));
  if (!texture->pixels) {
    BR_LOG_ERROR("Failed to allocate texture pixels: '%s'", filepath);
    free(texture);
    free(unfiltered_data);
    return NULL;
  }

  // Convert RGBA bytes to ARGB 32-bit integers
  for (size_t i = 0; i < num_pixels; i++) {
    uint8_t r = unfiltered_data[i * 4 + 0];
    uint8_t g = unfiltered_data[i * 4 + 1];
    uint8_t b = unfiltered_data[i * 4 + 2];
    uint8_t a = unfiltered_data[i * 4 + 3];

    texture->pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }
  free(unfiltered_data);

  BR_LOG_DEBUG("Successfully loaded texture: '%s'", filepath);

  return texture;
}

void br_texture_destroy(BrTexture *texture) {
  if (texture) {
    if (texture->pixels) {
      free(texture->pixels);
    }
    free(texture);
  }
}
