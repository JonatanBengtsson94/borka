#include "pch.h"

#include "borka_log.h"
#include "borka_texture.h"
#include "io/br_io.h"

#define ZLIB_COMPRESSION_METHOD_DEFLATE 8

#define MAX_CODE_LENGTH 15
#define MAX_CODE_LENGTH_SYMBOLS 19
#define MAX_LITERAL_LENGTH_SYMBOLS 286
#define MAX_DISTANCE_SYMBOLS 32

#define PNG_BYTES_PER_PIXEL_RGBA 4
#define PNG_COLOR_TYPE_RGBA 6
#define PNG_BIT_DEPTH_8 8

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

typedef struct {
  const uint8_t *buffer;
  size_t size;
  size_t byte_position;
  uint8_t bit_position;
} BitReader;

typedef struct {
  uint16_t code[286];
  uint8_t length[286];
  uint16_t max_bits;
  uint16_t table_size;
  uint16_t *lookup_table;
} HuffmanTable;

// --- PNG DEFLATE DECOMPRESSION ---

static bool parse_zlib_header(const uint8_t *data, size_t size) {
  struct {
    uint8_t compression_method;
    uint8_t compression_info;
    uint8_t fcheck;
    uint8_t fdict;
    uint8_t flevel;
  } header;
  if (size < 2) {
    BR_LOG_ERROR("Data to small to contain zlib header");
    return false;
  }
  uint8_t cmf = data[0];
  uint8_t flg = data[1];

  header.compression_method = cmf & 0x0F;
  header.compression_info = (cmf >> 4) & 0x0F;

  header.fcheck = flg & 0x1F;
  header.fdict = (flg >> 5) & 0x01;
  header.flevel = (flg >> 6) & 0x03;

  if (((uint16_t)cmf << 8 | flg) % 31 != 0) {
    BR_LOG_ERROR("Invalid fcheck checksum, header not valid");
    return false;
  }

  if (header.compression_method != ZLIB_COMPRESSION_METHOD_DEFLATE) {
    BR_LOG_ERROR("Only DEFLATE compression method is valid (got %u)",
                 header.compression_method);
    return false;
  }

  if (header.fdict != 0) {
    BR_LOG_ERROR("PNGs should not use preset dictionary");
    return false;
  }

  if (header.compression_info > 7) {
    BR_LOG_ERROR("Invalid compression info");
    return false;
  }

  BR_LOG_TRACE("Zlib header: compression_method: %u, compression_info: %u, "
               "fdict: %u, flevel: %u, fcheck: %u",
               header.compression_method, header.compression_info, header.fdict,
               header.flevel, header.fcheck);
  return true;
}

static bool has_bits(BitReader *bit_reader, int number_of_bits) {
  if (number_of_bits <= 0 || number_of_bits > 32) {
    BR_LOG_ERROR("Requested invalid number of bits: %d", number_of_bits);
    return false;
  }

  size_t total_bits = bit_reader->size * 8;
  size_t position = bit_reader->byte_position * 8 + bit_reader->bit_position;
  if (position + number_of_bits > total_bits) {
    BR_LOG_ERROR("Attempted to read/peek %d bits, but only %zu bits remain",
                 number_of_bits, total_bits - position);
    return false;
  }
  return true;
}

static int read_bit(BitReader *bit_reader) {
  int bit = (bit_reader->buffer[bit_reader->byte_position] >>
             bit_reader->bit_position) &
            1;
  bit_reader->bit_position++;
  if (bit_reader->bit_position == 8) {
    bit_reader->bit_position = 0;
    bit_reader->byte_position++;
  }
  return bit;
}

static bool read_bits(BitReader *bit_reader, int bits_to_read,
                      uint32_t *out_value) {
  if (!has_bits(bit_reader, bits_to_read)) {
    return false;
  }

  uint32_t value = 0;
  for (int i = 0; i < bits_to_read; i++) {
    value |= (read_bit(bit_reader) << i);
  }

  *out_value = value;
  return true;
}

static bool peek_bits(BitReader *bit_reader, int bits_to_peek,
                      uint32_t *out_value) {
  if (!has_bits(bit_reader, bits_to_peek)) {
    return false;
  }

  uint32_t value = 0;
  size_t byte_position = bit_reader->byte_position;
  uint8_t bit_position = bit_reader->bit_position;
  int bit_count = 0;

  while (bit_count < bits_to_peek) {
    int bits_left_in_byte = 8 - bit_position;
    int bits_to_take;
    if (bits_to_peek - bit_count < bits_left_in_byte) {
      bits_to_take = bits_to_peek - bit_count;
    } else {
      bits_to_take = bits_left_in_byte;
    }

    uint8_t mask = (1 << bits_to_take) - 1;
    value |= ((bit_reader->buffer[byte_position] >> bit_position) & mask)
             << bit_count;

    bit_count += bits_to_take;
    bit_position += bits_to_take;
    if (bit_position == 8) {
      bit_position = 0;
      byte_position++;
    }
  }

  *out_value = value;
  return true;
}

static bool decode_deflate_length(BitReader *bit_reader, uint32_t length_code,
                                  uint32_t *out_length) {
  if (length_code < 257 || length_code > 285) {
    BR_LOG_ERROR("Invalid length code: %u", length_code);
    return false;
  }

  const struct {
    uint8_t extra_bits;
    uint16_t base_length;
  } length_table[] = {
      {0, 3},   {0, 4},   {0, 5},   {0, 6},
      {0, 7},   {0, 8},   {0, 9},   {0, 10},  // 257-264
      {1, 11},  {1, 13},  {1, 15},  {1, 17},  // 265-268
      {2, 19},  {2, 23},  {2, 27},  {2, 31},  // 269-272
      {3, 35},  {3, 43},  {3, 51},  {3, 59},  // 273-276
      {4, 67},  {4, 83},  {4, 99},  {4, 115}, // 277-280
      {5, 131}, {5, 163}, {5, 195}, {5, 227}, // 281-284
      {0, 258}                                // 285
  };

  size_t index = length_code - 257;
  uint32_t extra_bits = length_table[index].extra_bits;
  uint32_t base_length = length_table[index].base_length;

  if (extra_bits == 0) {
    *out_length = base_length;
    return true;
  }

  uint32_t extra_value = 0;
  if (!read_bits(bit_reader, extra_bits, &extra_value)) {
    BR_LOG_ERROR("Failed to read extra length bits");
    return false;
  }

  *out_length = base_length + extra_value;
  return true;
}

static bool decode_deflate_distance(BitReader *bit_reader,
                                    uint32_t distance_code,
                                    uint32_t *out_distance) {
  if (distance_code > 29) {
    BR_LOG_ERROR("Invalid distance code: %u", distance_code);
    return false;
  }

  const struct {
    uint8_t extra_bits;
    uint16_t base_distance;
  } distance_table[] = {
      {0, 1},      {0, 2},      {0, 3}, {0, 4}, // 0-3
      {1, 5},      {1, 7},                      // 4-5
      {2, 9},      {2, 13},                     // 6-7
      {3, 17},     {3, 25},                     // 8-9
      {4, 33},     {4, 49},                     // 10-11
      {5, 65},     {5, 97},                     // 12-13
      {6, 129},    {6, 193},                    // 14-15
      {7, 257},    {7, 385},                    // 16-17
      {8, 513},    {8, 769},                    // 18-19
      {9, 1025},   {9, 1537},                   // 20-21
      {10, 2049},  {10, 3073},                  // 22-23
      {11, 4097},  {11, 6145},                  // 24-25
      {12, 8193},  {12, 12289},                 // 26-27
      {13, 16385}, {13, 24577}                  // 28-29
  };

  size_t index = distance_code;
  size_t extra_bits = distance_table[index].extra_bits;
  size_t base_distance = distance_table[index].base_distance;

  if (extra_bits == 0) {
    *out_distance = base_distance;
    return true;
  }

  uint32_t extra_value = 0;
  if (!read_bits(bit_reader, extra_bits, &extra_value)) {
    BR_LOG_ERROR("Failed to read extra distance bits");
    return false;
  }

  *out_distance = base_distance + extra_value;
  return true;
}

static uint32_t reverse_bits(uint32_t value, int number_of_bits) {
  uint32_t result = 0;
  for (int i = 0; i < number_of_bits; i++) {
    result = (result << 1) | (value & 1);
    value >>= 1;
  }
  return result;
}

static bool build_huffman_codes(HuffmanTable *table,
                                const uint8_t *symbol_lengths,
                                int number_of_symbols) {
  uint16_t codes_per_length[MAX_CODE_LENGTH + 1] = {0};
  table->max_bits = 0;

  for (int i = 0; i < number_of_symbols; i++) {
    uint8_t length = symbol_lengths[i];
    if (length > MAX_CODE_LENGTH) {
      BR_LOG_ERROR("Code length too large: %u", length);
      return false;
    }
    if (length > 0) {
      codes_per_length[length]++;
      if (length > table->max_bits) {
        table->max_bits = length;
      }
    }
    table->length[i] = length;
    BR_LOG_TRACE("Length %d: %d", i, table->length[i]);
  }

  if (table->max_bits == 0) {
    BR_LOG_ERROR("No non-zero code lengths found");
    return false;
  }

  uint16_t next_code_value[MAX_CODE_LENGTH + 1];
  uint16_t code = 0;
  codes_per_length[0] = 0;
  for (int bits = 1; bits <= table->max_bits; bits++) {
    code = (code + codes_per_length[bits - 1]) << 1;
    next_code_value[bits] = code;
  }

  for (int i = 0; i < number_of_symbols; i++) {
    int length = table->length[i];
    if (length != 0) {
      table->code[i] = next_code_value[length];
      table->code[i] = reverse_bits(table->code[i], length);
      BR_LOG_TRACE("Code %d: %d", i, table->code[i]);
      next_code_value[length]++;
    }
  }

  table->table_size = 1 << table->max_bits;
  table->lookup_table = (uint16_t *)calloc(table->table_size, sizeof(uint16_t));
  if (!table->lookup_table) {
    BR_LOG_ERROR("Failed to allocate lookup table");
    return false;
  }

  for (int symbol = 0; symbol < number_of_symbols; symbol++) {
    int length = table->length[symbol];
    if (length != 0) {
      uint16_t code_value = table->code[symbol];
      uint16_t step = 1 << (table->max_bits - length);
      uint16_t combined_value = (symbol << 4) | length;

      for (uint16_t i = 0; i < step; i++) {
        uint16_t table_index = code_value | (i << length);
        table->lookup_table[table_index] = combined_value;
      }
    }
  }

  return true;
}

static uint32_t decode_symbol(BitReader *bit_reader,
                              const HuffmanTable *table) {
  uint32_t peeked_bits = 0;
  if (!peek_bits(bit_reader, table->max_bits, &peeked_bits))
    return UINT32_MAX;

  uint16_t combined = table->lookup_table[peeked_bits];
  uint32_t symbol = combined >> 4;
  uint32_t length = combined & 0xF;

  if (length == 0) {
    BR_LOG_ERROR("Decoded symbol has length 0");
    return UINT32_MAX;
  }

  uint32_t throwaway;
  if (!read_bits(bit_reader, length, &throwaway))
    return UINT32_MAX;

  return symbol;
}

static bool decode_btype00(BitReader *bit_reader, uint8_t *out_data,
                           size_t out_size, size_t *output_position) {
  if (bit_reader->bit_position != 0) {
    bit_reader->bit_position = 0;
    bit_reader->byte_position++;
  }

  if (bit_reader->byte_position + 4 > bit_reader->size) {
    BR_LOG_ERROR("Not enough data for LEN/NLEN");
    return false;
  }

  uint16_t len = bit_reader->buffer[bit_reader->byte_position] |
                 (bit_reader->buffer[bit_reader->byte_position + 1] << 8);
  uint16_t nlen = bit_reader->buffer[bit_reader->byte_position + 2] |
                  (bit_reader->buffer[bit_reader->byte_position + 3] << 8);

  if ((len ^ nlen) != 0xFFFF) {
    BR_LOG_ERROR("LEN/NLEN mismatch");
    return false;
  }
  bit_reader->byte_position += 4; // advance past len + nlen

  if (bit_reader->byte_position + len > bit_reader->size) {
    BR_LOG_ERROR("Not enough data for block");
    return false;
  }

  if (*output_position + len > out_size) {
    BR_LOG_ERROR("Output buffer overflow: trying to write %u bytes at "
                 "position %zu (max: %zu)",
                 len, *output_position, out_size);
    return false;
  }

  memcpy(out_data, bit_reader->buffer + bit_reader->byte_position, len);
  bit_reader->byte_position += len; // advance past block
  *output_position += len;

  return true;
}

static bool decode_btype01(BitReader *bit_reader, uint8_t *out_data,
                           size_t out_size, size_t *output_position) {
  while (1) {
    uint32_t raw_value = 0;
    if (!peek_bits(bit_reader, 9, &raw_value))
      return false;
    uint32_t symbol = UINT32_MAX;
    uint32_t code = UINT32_MAX;

    uint32_t value_7bit = reverse_bits(raw_value & 0x7F, 7);
    if (value_7bit <= 23) {
      read_bits(bit_reader, 7, &code);
      symbol = reverse_bits(code, 7) + 256;
    }

    else {
      uint32_t value_8bit = reverse_bits(raw_value & 0xFF, 8);
      if (value_8bit >= 48 && value_8bit <= 191) {
        read_bits(bit_reader, 8, &code);
        symbol = reverse_bits(code, 8) - 48;
      } else if (value_8bit >= 192 && value_8bit <= 199) {
        read_bits(bit_reader, 8, &code);
        symbol = reverse_bits(code, 8) - 192 + 280;
      }

      else {
        read_bits(bit_reader, 9, &code);
        symbol = reverse_bits(code, 9) - 400 + 144;
      }
    }

    if (symbol <= 255) {
      BR_LOG_TRACE("Literal: %u", symbol);
      if (*output_position + 1 >= out_size) {
        BR_LOG_ERROR("Output buffer overflow: trying to write %u bytes at "
                     "position %zu (max: %zu)",
                     1, *output_position, out_size);
        return false;
      }
      out_data[*output_position] = (uint8_t)symbol;
      (*output_position)++;
      continue;
    }

    if (symbol == 256) {
      BR_LOG_TRACE("End of Block: %u", symbol);
      break;
    }

    if (symbol >= 257 && symbol <= 285) {
      uint32_t length = 0;
      if (!decode_deflate_length(bit_reader, symbol, &length))
        return false;

      uint32_t distance_code = 0;
      if (!read_bits(bit_reader, 5, &distance_code))
        return false;
      distance_code = reverse_bits(distance_code, 5);

      uint32_t distance = 0;
      if (!decode_deflate_distance(bit_reader, distance_code, &distance))
        return false;
      BR_LOG_TRACE("Length: %u, Distance: %u", length, distance);

      if (*output_position + length > out_size) {
        BR_LOG_ERROR("Output buffer overflow: trying to write %u bytes at "
                     "position %zu (max: %zu)",
                     length, *output_position, out_size);
        return false;
      }
      if (distance == 0 || *output_position < distance) {
        BR_LOG_ERROR("Invalid backward distance: %u (Current position: %zu)",
                     distance, *output_position);
        return false;
      }

      size_t copy_start = *output_position - distance;
      for (uint32_t i = 0; i < length; i++) {
        out_data[*output_position] = out_data[copy_start + i];
        (*output_position)++;
      }
      continue;
    }

    BR_LOG_ERROR("Invalid symbol: %u", symbol);
    return false;
  }

  return true;
}

static bool decode_btype10(BitReader *bit_reader, uint8_t *out_data,
                           size_t out_size, size_t *output_position) {
  bool success = false;
  const uint8_t hclen_order[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                   11, 4,  12, 3, 13, 2, 14, 1, 15};

  // Read header
  uint32_t hlit = 0;
  if (!read_bits(bit_reader, 5, &hlit))
    goto cleanup;
  int number_literal_length_symbols = hlit + 257;
  uint32_t hdist = 0;
  if (!read_bits(bit_reader, 5, &hdist))
    goto cleanup;
  int number_distance_symbols = hdist + 1;
  uint32_t hclen = 0;
  if (!read_bits(bit_reader, 4, &hclen))
    goto cleanup;
  int number_code_length_symbols = hclen + 4;
  BR_LOG_TRACE(
      "HLIT: %u (%d symbols), HDIST: %u (%d symbols), HCLEN: %u (%d symbols)",
      hlit, number_literal_length_symbols, hdist, number_distance_symbols,
      hclen, number_code_length_symbols);

  // Read code length symbol lengths
  uint8_t code_length_symbol_lengths[19] = {0};
  for (int i = 0; i < number_code_length_symbols; i++) {
    uint32_t length = 0;
    if (!read_bits(bit_reader, 3, &length))
      goto cleanup;
    code_length_symbol_lengths[hclen_order[i]] = (uint8_t)length;
  }

  // Build code length table
  HuffmanTable code_length_table = {0};
  if (!build_huffman_codes(&code_length_table, code_length_symbol_lengths,
                           MAX_CODE_LENGTH_SYMBOLS)) {
    BR_LOG_ERROR("Failed to build code length table");
    goto cleanup;
  }

  // Decode literal/length and distance code lengths
  uint8_t literal_length_symbol_lengths[MAX_LITERAL_LENGTH_SYMBOLS] = {0};
  uint8_t distance_symbol_lengths[MAX_DISTANCE_SYMBOLS] = {0};
  int total_lengths_to_decode =
      number_literal_length_symbols + number_distance_symbols;
  BR_LOG_TRACE("Lengths to decode: %u", total_lengths_to_decode);

  uint8_t *current_lengths = literal_length_symbol_lengths;
  int current_max = number_literal_length_symbols;
  int i = 0;
  int current_lengths_decoded = 0;
  uint8_t previous_length = 0;

  while (current_lengths_decoded < total_lengths_to_decode) {
    if (current_lengths_decoded == number_literal_length_symbols) {
      current_lengths = distance_symbol_lengths;
      current_max = number_distance_symbols;
      i = 0;
    }

    uint32_t symbol = decode_symbol(bit_reader, &code_length_table);
    if (symbol == UINT32_MAX) {
      goto cleanup;
    }

    if (symbol < 16) {
      current_lengths[i] = (uint8_t)symbol;
      previous_length = (uint8_t)symbol;
      i++;
    } else if (symbol == 16) {
      uint32_t repeat_count = 0;
      if (!read_bits(bit_reader, 2, &repeat_count))
        goto cleanup;
      repeat_count += 3;

      if (i == 0) {
        BR_LOG_ERROR("Symbol 16 used without a previous length");
        goto cleanup;
      }
      for (uint32_t j = 0; j < repeat_count && i < current_max; j++) {
        current_lengths[i++] = previous_length;
      }
    } else if (symbol == 17) {
      uint32_t repeat_count = 0;
      if (!read_bits(bit_reader, 3, &repeat_count))
        goto cleanup;
      repeat_count += 3;
      for (uint32_t j = 0; j < repeat_count && i < current_max; j++) {
        current_lengths[i++] = 0;
      }
    } else if (symbol == 18) {
      uint32_t repeat_count = 0;
      if (!read_bits(bit_reader, 7, &repeat_count))
        goto cleanup;
      repeat_count += 11;
      for (uint32_t j = 0; j < repeat_count && i < current_max; j++) {
        current_lengths[i++] = 0;
      }
    } else {
      BR_LOG_ERROR("Invalid code length symbol: %u", symbol);
      goto cleanup;
    }

    if (current_lengths == literal_length_symbol_lengths) {
      current_lengths_decoded = i;
    } else {
      current_lengths_decoded = number_literal_length_symbols + i;
    }

    if (i > current_max) {
      BR_LOG_WARN("Code length decoding overshot boundary. Clamping");
      i = current_max;
    }
  }

  HuffmanTable literal_length_table = {0};
  if (!build_huffman_codes(&literal_length_table, literal_length_symbol_lengths,
                           number_literal_length_symbols)) {
    BR_LOG_ERROR("Failed to build literal/length table");
    goto cleanup;
  }

  HuffmanTable distance_table = {0};
  if (!build_huffman_codes(&distance_table, distance_symbol_lengths,
                           number_distance_symbols)) {
    BR_LOG_ERROR("Failed to build distance table");
    goto cleanup;
  }

  while (1) {
    uint32_t symbol = decode_symbol(bit_reader, &literal_length_table);

    if (symbol == UINT32_MAX) {
      BR_LOG_ERROR("Failed to decode literal/length symbol");
      goto cleanup;
    }

    if (symbol < 256) {
      if (*output_position >= out_size) {
        BR_LOG_ERROR("Output buffer overflow: trying to write %u bytes at "
                     "position %zu (max: %zu)",
                     1, *output_position, out_size);
        goto cleanup;
      }
      out_data[*(output_position)] = (uint8_t)symbol;
      (*output_position)++;
      continue;
    }

    if (symbol == 256) {
      BR_LOG_TRACE("End of Block: %u", symbol);
      break;
    }

    if (symbol >= 257 && symbol <= 285) {
      uint32_t length_code = symbol;
      uint32_t length = 0;
      if (!decode_deflate_length(bit_reader, length_code, &length))
        goto cleanup;

      uint32_t distance_code = decode_symbol(bit_reader, &distance_table);
      if (distance_code == UINT32_MAX) {
        BR_LOG_ERROR("Failed to decode distance symbol");
        goto cleanup;
      }

      uint32_t distance = 0;
      if (!decode_deflate_distance(bit_reader, distance_code, &distance))
        goto cleanup;
      BR_LOG_TRACE("Length: %u, Distance: %u", length, distance);

      if (*output_position + length > out_size) {
        BR_LOG_ERROR("Output buffer overflow: trying to write %u bytes at "
                     "position %zu (max: %zu)",
                     length, *output_position, out_size);
        goto cleanup;
      }
      if (distance == 0 || *output_position < distance) {
        BR_LOG_ERROR("Invalid backward distance: %u (Current position: %zu)",
                     distance, *output_position);
        goto cleanup;
      }

      size_t copy_start = *output_position - distance;
      for (uint32_t k = 0; k < length; k++) {
        out_data[*output_position] = out_data[copy_start + k];
        (*output_position)++;
      }
      continue;
    }

    BR_LOG_ERROR("Invalid symbol: %u", symbol);
    return false;
  }

  success = true;

cleanup:
  if (code_length_table.lookup_table)
    free(code_length_table.lookup_table);
  if (literal_length_table.lookup_table)
    free(literal_length_table.lookup_table);
  if (distance_table.lookup_table)
    free(distance_table.lookup_table);
  return success;
}

static bool decompress_data(const uint8_t *compressed_data,
                            size_t compressed_size, size_t uncompressed_size,
                            uint8_t *target) {
  if (!parse_zlib_header(compressed_data, compressed_size)) {
    BR_LOG_ERROR("Invalid Zlib header");
    return false;
  }

  size_t output_position = 0;
  BitReader bit_reader = {.byte_position = 0,
                          .bit_position = 0,
                          .size = compressed_size - 2,
                          .buffer = compressed_data + 2};
  uint32_t bfinal;
  uint32_t btype;
  do {
    if (!read_bits(&bit_reader, 1, &bfinal)) {
      BR_LOG_ERROR("Failed to read bits");
      return false;
    }
    if (!read_bits(&bit_reader, 2, &btype)) {
      BR_LOG_ERROR("Failed to read bits");
      return false;
    }

    switch (btype) {
    case 0:
      BR_LOG_TRACE("BTYPE00, No compression");
      if (!decode_btype00(&bit_reader, target, uncompressed_size,
                          &output_position)) {
        BR_LOG_ERROR("Failed to decompress BTYPE00 block");
        return false;
      }
      BR_LOG_TRACE("Decompressed BTYPE00 correctly");
      break;

    case 1:
      BR_LOG_TRACE("BTYPE 01, Fixed huffman codes");
      if (!decode_btype01(&bit_reader, target, uncompressed_size,
                          &output_position)) {
        BR_LOG_ERROR("Failed to decompress BTYPE01 block");
        return false;
      }
      BR_LOG_TRACE("Decompressed BTYPE01 correctly");
      break;

    case 2:
      BR_LOG_TRACE("BTYPE 10, Dynamic huffman codes");
      if (!decode_btype10(&bit_reader, target, uncompressed_size,
                          &output_position)) {
        BR_LOG_ERROR("Failed to decompress BTYPE10 block");
        return false;
      }
      BR_LOG_TRACE("Decompressed BTYPE10 correctly");
      break;

    default:
      BR_LOG_ERROR("Invalid BTYPE (got %u)", btype);
      return false;
    }
  } while (bfinal != 1);

  return true;
}

// --- PNG DECODER ---

static uint32_t read_u32_be(const uint8_t *data) {
  return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
         ((uint32_t)data[2] << 8) | ((uint32_t)data[3]);
}

static bool validate_png_signature(const uint8_t *data, size_t file_size,
                                   size_t *offset) {
  const uint8_t PNG_SIGNATURE[8] = {0x89, 0x50, 0x4E, 0x47,
                                    0x0D, 0x0A, 0x1A, 0x0A};

  if (file_size < sizeof(PNG_SIGNATURE)) {
    BR_LOG_ERROR("File is too small for PNG signature");
    return false;
  }

  if (memcmp(data, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != 0) {
    BR_LOG_ERROR("Invalid PNG signature");
    return false;
  }
  *offset += sizeof(PNG_SIGNATURE);

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

  BR_LOG_TRACE(
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
      BR_LOG_TRACE("Found IEND chunk");
      break;
    }
    if (type == CHUNK_IDAT) {
      BR_LOG_TRACE("Found IDAT chunk: %u bytes (total: %zu)", length,
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

static bool unfilter_data(const uint8_t *filtered_data, uint32_t width,
                          uint32_t height, uint8_t *target) {
  enum {
    FILTER_NONE = 0,
    FILTER_SUB = 1,
    FILTER_UP = 2,
    FILTER_AVERAGE = 3,
    FILTER_PAETH = 4,
  };
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
      BR_LOG_TRACE("Filter method none");
      memcpy(dst_row, scanline, stride);
      break;

    case FILTER_SUB:
      BR_LOG_TRACE("Filter method sub");
      for (size_t x = 0; x < stride; x++) {
        uint8_t left = 0;
        if (x >= PNG_BYTES_PER_PIXEL_RGBA) {
          left = dst_row[x - PNG_BYTES_PER_PIXEL_RGBA];
        }
        dst_row[x] = scanline[x] + left;
      }
      break;

    case FILTER_UP:
      BR_LOG_TRACE("Filter method up");
      for (size_t x = 0; x < stride; x++) {
        uint8_t above = 0;
        if (prev_row) {
          above = prev_row[x];
        }
        dst_row[x] = scanline[x] + above;
      }
      break;

    case FILTER_AVERAGE:
      BR_LOG_TRACE("Filter method average");
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
      BR_LOG_TRACE("Filter method paeth");
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
                         BrVec2 size) {
  size_t total_pixels = size.x * size.y;
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

BrTexture *br_texture_create(const char *filepath) {
  // Read file
  size_t file_size;
  uint8_t *file_data = read_entire_file(filepath, &file_size);
  if (!file_data) {
    BR_LOG_ERROR("Failed to read file: '%s'", filepath);
    return NULL;
  }
  size_t offset = 0;

  // Read and validate signature
  if (!validate_png_signature(file_data, file_size, &offset)) {
    BR_LOG_ERROR("Could not validate png signature: '%s'", filepath);
    free(file_data);
    return NULL;
  }

  // Read and validate ihdr
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
  BR_LOG_TRACE("Successfully collected compressed data: '%s'", filepath);

  size_t scanline_size =
      ihdr.width * PNG_BYTES_PER_PIXEL_RGBA + 1; // +1 filter byte
  size_t uncompressed_size = scanline_size * ihdr.height;

  // Decompress data
  uint8_t *uncompressed_data = malloc(uncompressed_size);
  if (!uncompressed_data) {
    BR_LOG_ERROR("Failed to allocate uncompressed data");
    free(compressed_data);
    return NULL;
  }

  if (!decompress_data(compressed_data, compressed_size, uncompressed_size,
                       uncompressed_data)) {
    BR_LOG_ERROR("Failed to decompress data: '%s'", filepath);
    free(compressed_data);
    free(uncompressed_data);
    return NULL;
  }
  free(compressed_data);
  BR_LOG_TRACE("Successfully decompressed data: '%s'", filepath);

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
    free(unfiltered_data);
    free(uncompressed_data);
    return NULL;
  }
  free(uncompressed_data);
  BR_LOG_TRACE("Successfully unfiltered data: '%s'", filepath);

  // Allocate BrTexture
  BrTexture *texture = malloc(sizeof(BrTexture));
  if (!texture) {
    free(unfiltered_data);
    BR_LOG_ERROR("Failed to allocate texture: '%s'", filepath);
    return NULL;
  }

  texture->size.x = ihdr.width;
  texture->size.y = ihdr.height;
  texture->pixels = malloc(unfiltered_size);
  if (!texture->pixels) {
    BR_LOG_ERROR("Failed to allocate texture pixels: '%s'", filepath);
    free(unfiltered_data);
    free(texture);
    return NULL;
  }

  rgba_to_argb(unfiltered_data, texture->pixels, texture->size);
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
