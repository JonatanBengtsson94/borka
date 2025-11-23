#include "borka.h"
#include "borka_texture.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define PNG_COLOR_TYPE_RGBA 6
#define PNG_BIT_DEPTH_8 8
#define PNG_BYTES_PER_PIXEL_RGBA 4

const uint8_t PNG_SIGNATURE[8] = {0x89, 0x50, 0x4E, 0x47,
                                  0x0D, 0x0A, 0x1A, 0x0A};

typedef enum {
  CHUNK_IHDR = 0x49484452,
  CHUNK_IDAT = 0x49444154,
  CHUNK_IEND = 0x49454E44,
} ChunkType;

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

static bool parse_ihdr(const uint8_t *data, size_t file_size, size_t *offset,
                       IHDR *out_ihdr) {
  size_t current_offset = *offset;

  if (current_offset + 8 > file_size) {
    BR_LOG_ERROR("PNG too small for IHDR chunk");
    return false;
  }

  uint32_t length = read_u32_be(data + current_offset);
  current_offset += 4;
  uint32_t type = read_u32_be(data + current_offset);
  current_offset += 4;

  if (type != CHUNK_IHDR) {
    BR_LOG_ERROR("First PNG chunk is not IHDR");
    return false;
  }

  if (length != 13) {
    BR_LOG_ERROR("IHDR must be 13 bytes (got %u)", length);
    return false;
  }

  if (current_offset + length + 4 > file_size) {
    BR_LOG_ERROR("IHDR extends past end of file");
    return false;
  }

  // Validate IHDR Chunk
  out_ihdr->width = read_u32_be(data + current_offset);
  out_ihdr->height = read_u32_be(data + current_offset + 4);
  out_ihdr->bit_depth = data[current_offset + 8];
  out_ihdr->color_type = data[current_offset + 9];
  out_ihdr->compression_type = data[current_offset + 10];
  out_ihdr->filter_method = data[current_offset + 11];
  out_ihdr->interlace_method = data[current_offset + 12];

  // Advance beyond IHDR + CRC
  current_offset += length + 4;
  *offset = current_offset;

  if (out_ihdr->width == 0 || out_ihdr->height == 0) {
    BR_LOG_ERROR("Invalid PNG dimensions: %ux%u", out_ihdr->width,
                 out_ihdr->height);
    return false;
  }

  if (out_ihdr->bit_depth != PNG_BIT_DEPTH_8) {
    BR_LOG_ERROR("Only 8-bit PNG supported (got %u-bit)", out_ihdr->bit_depth);
    return false;
  }

  if (out_ihdr->color_type != PNG_COLOR_TYPE_RGBA) {
    BR_LOG_ERROR("Only RGBA PNG supported (got color_type %u)",
                 out_ihdr->color_type);
    return false;
  }

  if (out_ihdr->interlace_method != 0) {
    BR_LOG_ERROR("Interlaced PNG not supported");
    return false;
  }

  if (out_ihdr->compression_type != 0 || out_ihdr->filter_method != 0) {
    BR_LOG_ERROR("Invalid PNG format");
    return false;
  }

  BR_LOG_DEBUG(
      "PNG IHDR: width: %u, height: %u, bit_depth: %u, color_type: %u, "
      "compression_type: %u, filter_method: %u, interlace_method: %u",
      out_ihdr->width, out_ihdr->height, out_ihdr->bit_depth,
      out_ihdr->color_type, out_ihdr->compression_type, out_ihdr->filter_method,
      out_ihdr->interlace_method);

  return true;
}

static uint8_t *collect_compressed_data(const uint8_t *data, size_t size,
                                        size_t offset, size_t *out_size) {
  size_t current_offset = offset;
  size_t total_length = 0;

  // Calculate size
  while (current_offset + 8 <= size) {
    uint32_t length = read_u32_be(data + current_offset);
    current_offset += 4;
    uint32_t type = read_u32_be(data + current_offset);
    current_offset += 4;

    if (type == CHUNK_IEND) {
      BR_LOG_DEBUG("Found IEND chunk");
      break;
    }
    if (type == CHUNK_IDAT) {
      BR_LOG_DEBUG("Found IDAT chunk: %u bytes (total: %zu)", length,
                   total_length + length);
      total_length += length;
    }
    current_offset += length + 4; // Data + CRC
  }

  uint8_t *buffer = malloc(total_length);
  if (!buffer) {
    BR_LOG_ERROR("Failed to allocate buffer");
    return NULL;
  }

  size_t write_pos = 0;
  current_offset = offset;
  while (current_offset + 8 <= size) {
    uint32_t length = read_u32_be(data + current_offset);
    current_offset += 4;
    uint32_t type = read_u32_be(data + current_offset);
    current_offset += 4;

    if (type == CHUNK_IEND)
      break;
    if (type == CHUNK_IDAT) {
      memcpy(buffer + write_pos, data + current_offset, length);
      write_pos += length;
    }
    current_offset += length + 4; // Data + CRC
  }

  *out_size = total_length;
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

static bool unfilter_data(const uint8_t *filtered_data, uint32_t width,
                          uint32_t height, uint8_t *target) {
  size_t stride = width * PNG_BYTES_PER_PIXEL_RGBA;
  size_t src_stride = stride + 1;

  for (uint32_t y = 0; y < height; y++) {
    uint8_t filter_type = filtered_data[y * src_stride];
    const uint8_t *scanline = &filtered_data[y * src_stride + 1];
    uint8_t *dst_row = &target[y * stride];

    uint8_t *prev_row = NULL;
    if (y > 0) {
      prev_row = &target[(y - 1) * stride];
    }

    switch (filter_type) {
    case FILTER_NONE:
      memcpy(dst_row, scanline, stride);
      break;

    case FILTER_SUB:
      for (size_t x = 0; x < stride; x++) {
        uint8_t left = 0;
        if (x >= PNG_BYTES_PER_PIXEL_RGBA) {
          left = dst_row[x - PNG_BYTES_PER_PIXEL_RGBA];
        }
        dst_row[x] = scanline[x] + left;
      }
      break;

    case FILTER_UP:
      for (size_t x = 0; x < stride; x++) {
        uint8_t above = 0;
        if (prev_row) {
          above = prev_row[x];
        }
        dst_row[x] = scanline[x] + above;
      }
      break;

    case FILTER_AVERAGE:
      for (size_t x = 0; x < stride; x++) {
        uint8_t left = 0;
        if (x >= PNG_BYTES_PER_PIXEL_RGBA) {
          left = dst_row[x - PNG_BYTES_PER_PIXEL_RGBA];
        }

        uint8_t above = 0;
        if (prev_row) {
          above = prev_row[x];
        }

        dst_row[x] = scanline[x] + ((left + above) >> 1);
      }
      break;

    case FILTER_PAETH:
      for (size_t x = 0; x < stride; x++) {
        uint8_t left = 0;
        if (x >= PNG_BYTES_PER_PIXEL_RGBA) {
          left = dst_row[x - PNG_BYTES_PER_PIXEL_RGBA];
        }

        uint8_t above = 0;
        if (prev_row) {
          above = prev_row[x];
        }

        uint8_t above_left = 0;
        if (prev_row && x >= PNG_BYTES_PER_PIXEL_RGBA) {
          above_left = prev_row[x - PNG_BYTES_PER_PIXEL_RGBA];
        }

        int p = (int)left + (int)above - (int)above_left;
        int pa = abs(p - (int)left);
        int pb = abs(p - (int)above);
        int pc = abs(p - (int)above_left);

        uint8_t paeth_predictor;
        if (pa <= pb && pa <= pc) {
          paeth_predictor = left;
        } else if (pb <= pc) {
          paeth_predictor = above;
        } else {
          paeth_predictor = above_left;
        }

        dst_row[x] = scanline[x] + paeth_predictor;
      }
      break;

    default:
      BR_LOG_ERROR("Unknown filter method (got %u)", filter_type);
      return false;
      break;
    }
  }
  return true;
}

static void rgba_to_argb(const uint8_t *src_rgba, uint32_t *dst_argb,
                         uint32_t width, uint32_t height) {
  size_t total_pixels = width * height;
  for (size_t i = 0; i < total_pixels; i++) {
    size_t offset = i * 4;
    uint8_t r = src_rgba[offset + 0];
    uint8_t g = src_rgba[offset + 1];
    uint8_t b = src_rgba[offset + 2];
    uint8_t a = src_rgba[offset + 3];

    dst_argb[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }
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
  IHDR ihdr;
  if (!parse_ihdr(file_data, file_size, &offset, &ihdr)) {
    BR_LOG_ERROR("Failed to parse ihdr: '%s'", filepath);
    free(file_data);
    return NULL;
  }

  // Read idat chunks
  size_t compressed_size = 0;
  uint8_t *compressed_data =
      collect_compressed_data(file_data, file_size, offset, &compressed_size);
  free(file_data);
  if (compressed_size == 0) {
    BR_LOG_ERROR("No IDAT chunks found: '%s'", filepath);
    free(compressed_data);
    return NULL;
  }

  size_t scanline_size =
      ihdr.width * PNG_BYTES_PER_PIXEL_RGBA + 1; // +1 filter byte
  size_t uncompressed_size = scanline_size * ihdr.height;

  // Decompress data
  uint8_t *uncompressed_data =
      decompress_data(compressed_data, compressed_size, uncompressed_size);
  free(compressed_data);
  if (!uncompressed_data) {
    BR_LOG_ERROR("Failed to decompress data: '%s'", filepath);
    return NULL;
  }

  // Unfilter data
  size_t unfiltered_size = PNG_BYTES_PER_PIXEL_RGBA * ihdr.width * ihdr.height;
  uint8_t *unfiltered_data = malloc(unfiltered_size);
  if (!unfiltered_data) {
    BR_LOG_ERROR("Failed to allocate unfiltered data: '%s'", filepath);
    free(uncompressed_data);
    return NULL;
  }
  if (!unfilter_data(uncompressed_data, ihdr.width, ihdr.height,
                     unfiltered_data)) {
    BR_LOG_ERROR("Failed to unfilter data: '%s'", filepath);
    free(uncompressed_data);
    return NULL;
  }
  free(uncompressed_data);

  // Allocate BrTexture
  BrTexture *texture = malloc(sizeof(BrTexture));
  if (!texture) {
    free(unfiltered_data);
    BR_LOG_ERROR("Failed to allocate texture: '%s'", filepath);
    return NULL;
  }

  texture->width = ihdr.width;
  texture->height = ihdr.height;
  texture->pixels = malloc(unfiltered_size);
  if (!texture->pixels) {
    BR_LOG_ERROR("Failed to allocate texture pixels: '%s'", filepath);
    free(unfiltered_data);
    free(texture);
    return NULL;
  }

  rgba_to_argb(unfiltered_data, texture->pixels, texture->width,
               texture->height);

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
