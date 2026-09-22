#include "br_flac.h"
#include "pch.h"

#include "borka_log.h"

// --- BITSTREAM READER ---

typedef struct {
  const uint8_t *buffer;
  size_t size;
  size_t byte_position;
  uint8_t bit_position;
} BitReader;

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
             (7 - bit_reader->bit_position)) &
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
    value = (value << 1) | (uint32_t)read_bit(bit_reader);
  }

  *out_value = value;
  return true;
}

// Reads a two's complement value and sign-extends it to a full 32 bits.
static bool read_bits_signed(BitReader *bit_reader, int bits_to_read,
                             int32_t *out_value) {
  uint32_t value = 0;
  if (!read_bits(bit_reader, bits_to_read, &value))
    return false;

  if (bits_to_read < 32 && (value & (1u << (bits_to_read - 1))))
    value |= ~((1u << bits_to_read) - 1u);

  *out_value = (int32_t)value;
  return true;
}

// Reads a unary-coded value: the number of 0 bits before the next 1 bit.
static bool read_unary(BitReader *bit_reader, uint32_t *out_value) {
  uint32_t zeros = 0;
  while (true) {
    if (!has_bits(bit_reader, 1))
      return false;
    if (read_bit(bit_reader) != 0)
      break;
    zeros++;
  }

  *out_value = zeros;
  return true;
}

static void align_to_byte(BitReader *bit_reader) {
  if (bit_reader->bit_position != 0) {
    bit_reader->bit_position = 0;
    bit_reader->byte_position++;
  }
}

static bool skip_bytes(BitReader *bit_reader, size_t bytes) {
  if (bit_reader->byte_position + bytes > bit_reader->size) {
    BR_LOG_ERROR("Attempted to skip %zu bytes past the end of the stream",
                 bytes);
    return false;
  }

  bit_reader->byte_position += bytes;
  return true;
}

// --- CRC ---

static uint8_t crc8(const uint8_t *data, size_t size) {
  uint8_t crc = 0;
  for (size_t i = 0; i < size; i++) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static uint16_t crc16(const uint8_t *data, size_t size) {
  uint16_t crc = 0;
  for (size_t i = 0; i < size; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x8005)
                           : (uint16_t)(crc << 1);
    }
  }
  return crc;
}

// --- FLAC METADATA ---

typedef enum {
  STREAMINFO = 0,
  PADDING = 1,
  APPLICATION = 2,
  SEEK_TABLE = 3,
  VORBIS_COMMENT = 4,
  CUESHEET = 5,
  PICTURE = 6,
} MetaDataBlockType;

typedef struct {
  uint16_t min_block_size;
  uint16_t max_block_size;
  uint32_t min_frame_size;
  uint32_t max_frame_size;
  uint32_t sample_rate;
  uint8_t channels;
  uint8_t bits_per_sample;
  uint64_t total_samples;
} StreamInfo;

// On-disk size of the STREAMINFO block's payload, in bytes (excludes the
// 4-byte metadata block header that precedes every metadata block).
#define FLAC_STREAMINFO_SIZE 34
#define FLAC_STREAMINFO_MD5_SIZE 16

typedef struct {
  bool is_last;
  MetaDataBlockType type;
  uint32_t length; // Payload length in bytes, not counting this header.
} MetadataBlockHeader;

static const uint8_t FLAC_SIGNATURE[4] = {0x66, 0x4C, 0x61, 0x43}; // "fLaC"

bool br_flac_can_load(const uint8_t *data, size_t size) {
  return size >= sizeof(FLAC_SIGNATURE) &&
         memcmp(data, FLAC_SIGNATURE, sizeof(FLAC_SIGNATURE)) == 0;
}

static bool parse_metadata_block_header(BitReader *bit_reader,
                                        MetadataBlockHeader *out_header) {
  uint32_t is_last = 0;
  if (!read_bits(bit_reader, 1, &is_last)) {
    BR_LOG_ERROR("Failed to read metadata block last-flag");
    return false;
  }

  uint32_t type = 0;
  if (!read_bits(bit_reader, 7, &type)) {
    BR_LOG_ERROR("Failed to read metadata block type");
    return false;
  }

  uint32_t length = 0;
  if (!read_bits(bit_reader, 24, &length)) {
    BR_LOG_ERROR("Failed to read metadata block length");
    return false;
  }

  out_header->is_last = is_last != 0;
  out_header->type = (MetaDataBlockType)type;
  out_header->length = length;
  return true;
}

static bool parse_streaminfo(BitReader *bit_reader, StreamInfo *out_stream_info,
                             bool *out_is_last) {
  MetadataBlockHeader header;
  if (!parse_metadata_block_header(bit_reader, &header)) {
    BR_LOG_ERROR("Failed to read STREAMINFO block header");
    return false;
  }

  if (header.type != STREAMINFO) {
    BR_LOG_ERROR("Expected STREAMINFO as the first metadata block, got "
                 "type %d",
                 header.type);
    return false;
  }

  if (header.length != FLAC_STREAMINFO_SIZE) {
    BR_LOG_ERROR("Unexpected STREAMINFO length: %u (expected %u)",
                 header.length, FLAC_STREAMINFO_SIZE);
    return false;
  }

  uint32_t value = 0;

  if (!read_bits(bit_reader, 16, &value))
    return false;
  out_stream_info->min_block_size = (uint16_t)value;

  if (!read_bits(bit_reader, 16, &value))
    return false;
  out_stream_info->max_block_size = (uint16_t)value;

  if (!read_bits(bit_reader, 24, &value))
    return false;
  out_stream_info->min_frame_size = value;

  if (!read_bits(bit_reader, 24, &value))
    return false;
  out_stream_info->max_frame_size = value;

  if (!read_bits(bit_reader, 20, &value))
    return false;
  out_stream_info->sample_rate = value;
  if (out_stream_info->sample_rate != BR_AUDIO_SAMPLE_RATE) {
    BR_LOG_ERROR("Unsupported FLAC: sample rate is %u, must be %d",
                 out_stream_info->sample_rate, BR_AUDIO_SAMPLE_RATE);
    return false;
  }

  // Channels and bits-per-sample are stored as (actual value - 1) on disk.
  if (!read_bits(bit_reader, 3, &value))
    return false;
  out_stream_info->channels = (uint8_t)value + 1;
  if (out_stream_info->channels != BR_AUDIO_CHANNELS) {
    BR_LOG_ERROR("Unsupported FLAC: must be mono");
    return false;
  }

  if (!read_bits(bit_reader, 5, &value))
    return false;
  out_stream_info->bits_per_sample = (uint8_t)value + 1;
  if (out_stream_info->bits_per_sample != BR_AUDIO_BITS_PER_SAMPLE) {
    BR_LOG_ERROR("Unsupported FLAC: must be %d-bit, got %u",
                 BR_AUDIO_BITS_PER_SAMPLE, out_stream_info->bits_per_sample);
    return false;
  }

  // total_samples is 36 bits, wider than a single read_bits() call
  // supports, so read it as a 4-bit high part and a 32-bit low part.
  uint32_t total_samples_high = 0;
  uint32_t total_samples_low = 0;
  if (!read_bits(bit_reader, 4, &total_samples_high))
    return false;
  if (!read_bits(bit_reader, 32, &total_samples_low))
    return false;
  out_stream_info->total_samples =
      ((uint64_t)total_samples_high << 32) | total_samples_low;

  // The block ends with a 128-bit MD5 of the decoded audio. Nothing verifies
  // it, so the bytes are only skipped to reach the next metadata block.
  if (!skip_bytes(bit_reader, FLAC_STREAMINFO_MD5_SIZE))
    return false;

  BR_LOG_TRACE("FLAC Streaminfo: min_block_size: %u, max_block_size: %u, "
               "min_frame_size: %u, max_frame_size: %u, sample_rate: %u, "
               "channels: %u, bits_per_sample: %u, total_samples: %llu",
               out_stream_info->min_block_size, out_stream_info->max_block_size,
               out_stream_info->min_frame_size, out_stream_info->max_frame_size,
               out_stream_info->sample_rate, out_stream_info->channels,
               out_stream_info->bits_per_sample,
               (unsigned long long)out_stream_info->total_samples);

  *out_is_last = header.is_last;
  return true;
}

// Walks past every metadata block after STREAMINFO. Their contents (seek
// tables, tags, padding, ...) carry nothing the decoder needs, so each one
// is skipped by the length its header declares.
static bool skip_remaining_metadata(BitReader *bit_reader, bool is_last) {
  while (!is_last) {
    MetadataBlockHeader header;
    if (!parse_metadata_block_header(bit_reader, &header))
      return false;

    if (!skip_bytes(bit_reader, header.length)) {
      BR_LOG_ERROR("Metadata block of type %d claims %u bytes, past the end "
                   "of the file",
                   header.type, header.length);
      return false;
    }

    BR_LOG_TRACE("Skipped metadata block: type: %d, length: %u", header.type,
                 header.length);
    is_last = header.is_last;
  }

  return true;
}

// --- FLAC RESIDUAL ---

#define RESIDUAL_METHOD_RICE 0
#define RESIDUAL_METHOD_RICE2 1

// Decodes the prediction residuals for one subframe into samples[], starting
// at index predictor_order (the earlier slots hold the warm-up samples).
static bool decode_residual(BitReader *bit_reader, uint32_t block_size,
                            uint32_t predictor_order, int32_t *samples) {
  uint32_t method = 0;
  if (!read_bits(bit_reader, 2, &method))
    return false;

  int parameter_bits;
  uint32_t escape_parameter;
  if (method == RESIDUAL_METHOD_RICE) {
    parameter_bits = 4;
    escape_parameter = 0x0F;
  } else if (method == RESIDUAL_METHOD_RICE2) {
    parameter_bits = 5;
    escape_parameter = 0x1F;
  } else {
    BR_LOG_ERROR("Reserved residual coding method: %u", method);
    return false;
  }

  uint32_t partition_order = 0;
  if (!read_bits(bit_reader, 4, &partition_order))
    return false;

  uint32_t partitions = 1u << partition_order;
  if (partitions > block_size || block_size % partitions != 0) {
    BR_LOG_ERROR("Block size %u does not divide into %u partitions", block_size,
                 partitions);
    return false;
  }

  uint32_t sample_index = predictor_order;
  for (uint32_t partition = 0; partition < partitions; partition++) {
    uint32_t partition_samples = block_size / partitions;
    if (partition == 0) {
      if (partition_samples < predictor_order) {
        BR_LOG_ERROR("Predictor order %u exceeds the first partition's %u "
                     "samples",
                     predictor_order, partition_samples);
        return false;
      }
      partition_samples -= predictor_order;
    }

    uint32_t parameter = 0;
    if (!read_bits(bit_reader, parameter_bits, &parameter))
      return false;

    // An all-ones parameter escapes to unencoded residuals of a fixed width.
    if (parameter == escape_parameter) {
      uint32_t raw_bits = 0;
      if (!read_bits(bit_reader, 5, &raw_bits))
        return false;

      for (uint32_t i = 0; i < partition_samples; i++) {
        int32_t residual = 0;
        if (raw_bits > 0 &&
            !read_bits_signed(bit_reader, (int)raw_bits, &residual))
          return false;
        samples[sample_index++] = residual;
      }
      continue;
    }

    for (uint32_t i = 0; i < partition_samples; i++) {
      uint32_t msbs = 0;
      if (!read_unary(bit_reader, &msbs))
        return false;

      uint32_t lsbs = 0;
      if (parameter > 0 && !read_bits(bit_reader, (int)parameter, &lsbs))
        return false;

      // Rice codes are zigzag folded, so the low bit carries the sign.
      uint32_t folded = (msbs << parameter) | lsbs;
      samples[sample_index++] = (folded & 1)
                                    ? -(int32_t)(folded >> 1) - 1
                                    : (int32_t)(folded >> 1);
    }
  }

  return true;
}

// --- FLAC SUBFRAME ---

#define SUBFRAME_CONSTANT 0
#define SUBFRAME_VERBATIM 1
#define SUBFRAME_FIXED_MIN 8
#define SUBFRAME_FIXED_MAX 12
#define SUBFRAME_LPC_MIN 32
#define FLAC_MAX_LPC_ORDER 32

// Fixed predictors are just the first four difference polynomials, so each
// order restores samples from the ones before it with constant coefficients.
static void restore_fixed(int32_t *samples, uint32_t block_size,
                          uint32_t order) {
  switch (order) {
  case 1:
    for (uint32_t i = 1; i < block_size; i++)
      samples[i] += samples[i - 1];
    break;
  case 2:
    for (uint32_t i = 2; i < block_size; i++)
      samples[i] += 2 * samples[i - 1] - samples[i - 2];
    break;
  case 3:
    for (uint32_t i = 3; i < block_size; i++)
      samples[i] += 3 * samples[i - 1] - 3 * samples[i - 2] + samples[i - 3];
    break;
  case 4:
    for (uint32_t i = 4; i < block_size; i++)
      samples[i] += 4 * samples[i - 1] - 6 * samples[i - 2] +
                    4 * samples[i - 3] - samples[i - 4];
    break;
  default: // Order 0 predicts nothing; the residuals are the samples.
    break;
  }
}

static bool decode_fixed_subframe(BitReader *bit_reader, uint32_t block_size,
                                  uint8_t sample_bits, uint32_t order,
                                  int32_t *samples) {
  if (order > block_size) {
    BR_LOG_ERROR("Fixed order %u exceeds block size %u", order, block_size);
    return false;
  }

  for (uint32_t i = 0; i < order; i++) {
    if (!read_bits_signed(bit_reader, sample_bits, &samples[i]))
      return false;
  }

  if (!decode_residual(bit_reader, block_size, order, samples))
    return false;

  restore_fixed(samples, block_size, order);
  return true;
}

static bool decode_lpc_subframe(BitReader *bit_reader, uint32_t block_size,
                                uint8_t sample_bits, uint32_t order,
                                int32_t *samples) {
  if (order > block_size) {
    BR_LOG_ERROR("LPC order %u exceeds block size %u", order, block_size);
    return false;
  }

  for (uint32_t i = 0; i < order; i++) {
    if (!read_bits_signed(bit_reader, sample_bits, &samples[i]))
      return false;
  }

  uint32_t precision_bits = 0;
  if (!read_bits(bit_reader, 4, &precision_bits))
    return false;
  if (precision_bits == 0x0F) {
    BR_LOG_ERROR("Invalid LPC coefficient precision");
    return false;
  }
  int coefficient_bits = (int)precision_bits + 1;

  int32_t shift = 0;
  if (!read_bits_signed(bit_reader, 5, &shift))
    return false;
  if (shift < 0) {
    BR_LOG_ERROR("Invalid negative LPC shift: %d", shift);
    return false;
  }

  int32_t coefficients[FLAC_MAX_LPC_ORDER];
  for (uint32_t i = 0; i < order; i++) {
    if (!read_bits_signed(bit_reader, coefficient_bits, &coefficients[i]))
      return false;
  }

  if (!decode_residual(bit_reader, block_size, order, samples))
    return false;

  for (uint32_t i = order; i < block_size; i++) {
    int64_t prediction = 0;
    for (uint32_t j = 0; j < order; j++)
      prediction += (int64_t)coefficients[j] * samples[i - 1 - j];
    samples[i] += (int32_t)(prediction >> shift);
  }

  return true;
}

static bool decode_subframe(BitReader *bit_reader, uint32_t block_size,
                            uint8_t bits_per_sample, int32_t *samples) {
  uint32_t padding = 0;
  if (!read_bits(bit_reader, 1, &padding))
    return false;
  if (padding != 0) {
    BR_LOG_ERROR("Invalid subframe header: padding bit is set");
    return false;
  }

  uint32_t type = 0;
  if (!read_bits(bit_reader, 6, &type))
    return false;

  uint32_t has_wasted_bits = 0;
  if (!read_bits(bit_reader, 1, &has_wasted_bits))
    return false;

  // Wasted bits are trailing zeroes the encoder stripped from every sample
  // in this subframe; they are shifted back in once the samples are decoded.
  uint32_t wasted_bits = 0;
  if (has_wasted_bits) {
    uint32_t zeros = 0;
    if (!read_unary(bit_reader, &zeros))
      return false;
    wasted_bits = zeros + 1;
  }

  if (wasted_bits >= bits_per_sample) {
    BR_LOG_ERROR("Subframe wastes %u of %u bits per sample", wasted_bits,
                 bits_per_sample);
    return false;
  }
  uint8_t sample_bits = (uint8_t)(bits_per_sample - wasted_bits);

  if (type == SUBFRAME_CONSTANT) {
    int32_t constant = 0;
    if (!read_bits_signed(bit_reader, sample_bits, &constant))
      return false;
    for (uint32_t i = 0; i < block_size; i++)
      samples[i] = constant;
  } else if (type == SUBFRAME_VERBATIM) {
    for (uint32_t i = 0; i < block_size; i++) {
      if (!read_bits_signed(bit_reader, sample_bits, &samples[i]))
        return false;
    }
  } else if (type >= SUBFRAME_FIXED_MIN && type <= SUBFRAME_FIXED_MAX) {
    if (!decode_fixed_subframe(bit_reader, block_size, sample_bits,
                               type - SUBFRAME_FIXED_MIN, samples))
      return false;
  } else if (type >= SUBFRAME_LPC_MIN) {
    if (!decode_lpc_subframe(bit_reader, block_size, sample_bits,
                             type - SUBFRAME_LPC_MIN + 1, samples))
      return false;
  } else {
    BR_LOG_ERROR("Reserved subframe type: %u", type);
    return false;
  }

  // Shifted through unsigned because left-shifting a negative signed value
  // is undefined, and decoded samples are routinely negative.
  if (wasted_bits > 0) {
    for (uint32_t i = 0; i < block_size; i++)
      samples[i] = (int32_t)((uint32_t)samples[i] << wasted_bits);
  }

  return true;
}

// --- FLAC FRAME ---

#define FLAC_FRAME_SYNC_CODE 0x3FFE // 14 bits

// Conservative lower bound on the encoded size of a single frame, in bytes.
#define FLAC_MIN_FRAME_SIZE 8

typedef struct {
  uint32_t block_size;
  uint32_t sample_rate;
  uint8_t bits_per_sample;
} FrameHeader;

// Frame and sample numbers use a UTF-8 style encoding widened to 36 bits:
// the leading byte's high bits give the length, the rest carry the payload.
static bool read_utf8_coded_number(BitReader *bit_reader, uint64_t *out_value) {
  uint32_t first = 0;
  if (!read_bits(bit_reader, 8, &first))
    return false;

  int continuation_bytes;
  uint64_t value;
  if ((first & 0x80) == 0x00) {
    *out_value = first;
    return true;
  } else if ((first & 0xE0) == 0xC0) {
    continuation_bytes = 1;
    value = first & 0x1F;
  } else if ((first & 0xF0) == 0xE0) {
    continuation_bytes = 2;
    value = first & 0x0F;
  } else if ((first & 0xF8) == 0xF0) {
    continuation_bytes = 3;
    value = first & 0x07;
  } else if ((first & 0xFC) == 0xF8) {
    continuation_bytes = 4;
    value = first & 0x03;
  } else if ((first & 0xFE) == 0xFC) {
    continuation_bytes = 5;
    value = first & 0x01;
  } else if (first == 0xFE) {
    continuation_bytes = 6;
    value = 0;
  } else {
    BR_LOG_ERROR("Invalid UTF-8 coded number lead byte: 0x%02X", first);
    return false;
  }

  for (int i = 0; i < continuation_bytes; i++) {
    uint32_t next = 0;
    if (!read_bits(bit_reader, 8, &next))
      return false;
    if ((next & 0xC0) != 0x80) {
      BR_LOG_ERROR("Invalid UTF-8 coded number continuation byte: 0x%02X",
                   next);
      return false;
    }
    value = (value << 6) | (next & 0x3F);
  }

  *out_value = value;
  return true;
}

static bool parse_frame_header(BitReader *bit_reader,
                               const StreamInfo *stream_info,
                               FrameHeader *out_header) {
  size_t header_start = bit_reader->byte_position;

  uint32_t sync_code = 0;
  if (!read_bits(bit_reader, 14, &sync_code))
    return false;
  if (sync_code != FLAC_FRAME_SYNC_CODE) {
    BR_LOG_ERROR("Invalid frame sync code: 0x%04X", sync_code);
    return false;
  }

  uint32_t reserved = 0;
  if (!read_bits(bit_reader, 1, &reserved))
    return false;
  if (reserved != 0) {
    BR_LOG_ERROR("Invalid frame header: reserved bit is set");
    return false;
  }

  uint32_t blocking_strategy = 0;
  if (!read_bits(bit_reader, 1, &blocking_strategy))
    return false;

  uint32_t block_size_bits = 0;
  if (!read_bits(bit_reader, 4, &block_size_bits))
    return false;

  uint32_t sample_rate_bits = 0;
  if (!read_bits(bit_reader, 4, &sample_rate_bits))
    return false;

  uint32_t channel_assignment = 0;
  if (!read_bits(bit_reader, 4, &channel_assignment))
    return false;

  uint32_t sample_size_bits = 0;
  if (!read_bits(bit_reader, 3, &sample_size_bits))
    return false;

  if (!read_bits(bit_reader, 1, &reserved))
    return false;
  if (reserved != 0) {
    BR_LOG_ERROR("Invalid frame header: trailing reserved bit is set");
    return false;
  }

  // Frame number for fixed block sizes, sample number for variable ones.
  // Decoding is sequential, so the value itself is not needed.
  uint64_t coded_number = 0;
  if (!read_utf8_coded_number(bit_reader, &coded_number))
    return false;

  // Block sizes 0b0110 and 0b0111 defer the real value to after the header.
  uint32_t block_size = 0;
  int deferred_block_size_bits = 0;
  switch (block_size_bits) {
  case 0x0:
    BR_LOG_ERROR("Reserved frame block size");
    return false;
  case 0x1:
    block_size = 192;
    break;
  case 0x2:
  case 0x3:
  case 0x4:
  case 0x5:
    block_size = 576u << (block_size_bits - 2);
    break;
  case 0x6:
    deferred_block_size_bits = 8;
    break;
  case 0x7:
    deferred_block_size_bits = 16;
    break;
  default:
    block_size = 256u << (block_size_bits - 8);
    break;
  }

  // Sample rates 0b1100-0b1110 likewise defer to after the header.
  uint32_t sample_rate = 0;
  int deferred_sample_rate_bits = 0;
  uint32_t deferred_sample_rate_scale = 1;
  switch (sample_rate_bits) {
  case 0x0:
    sample_rate = stream_info->sample_rate;
    break;
  case 0x1:
    sample_rate = 88200;
    break;
  case 0x2:
    sample_rate = 176400;
    break;
  case 0x3:
    sample_rate = 192000;
    break;
  case 0x4:
    sample_rate = 8000;
    break;
  case 0x5:
    sample_rate = 16000;
    break;
  case 0x6:
    sample_rate = 22050;
    break;
  case 0x7:
    sample_rate = 24000;
    break;
  case 0x8:
    sample_rate = 32000;
    break;
  case 0x9:
    sample_rate = 44100;
    break;
  case 0xA:
    sample_rate = 48000;
    break;
  case 0xB:
    sample_rate = 96000;
    break;
  case 0xC:
    deferred_sample_rate_bits = 8;
    deferred_sample_rate_scale = 1000;
    break;
  case 0xD:
    deferred_sample_rate_bits = 16;
    break;
  case 0xE:
    deferred_sample_rate_bits = 16;
    deferred_sample_rate_scale = 10;
    break;
  default:
    BR_LOG_ERROR("Invalid frame sample rate");
    return false;
  }

  uint8_t bits_per_sample = 0;
  switch (sample_size_bits) {
  case 0x0:
    bits_per_sample = stream_info->bits_per_sample;
    break;
  case 0x1:
    bits_per_sample = 8;
    break;
  case 0x2:
    bits_per_sample = 12;
    break;
  case 0x4:
    bits_per_sample = 16;
    break;
  case 0x5:
    bits_per_sample = 20;
    break;
  case 0x6:
    bits_per_sample = 24;
    break;
  case 0x7:
    bits_per_sample = 32;
    break;
  default:
    BR_LOG_ERROR("Reserved frame sample size");
    return false;
  }

  // Assignments 8-10 are the stereo decorrelation modes, 11-15 are reserved.
  if (channel_assignment != 0) {
    BR_LOG_ERROR("Unsupported FLAC: must be mono (channel assignment %u)",
                 channel_assignment);
    return false;
  }

  if (deferred_block_size_bits > 0) {
    uint32_t value = 0;
    if (!read_bits(bit_reader, deferred_block_size_bits, &value))
      return false;
    block_size = value + 1;
  }

  if (deferred_sample_rate_bits > 0) {
    uint32_t value = 0;
    if (!read_bits(bit_reader, deferred_sample_rate_bits, &value))
      return false;
    sample_rate = value * deferred_sample_rate_scale;
  }

  uint8_t computed_crc =
      crc8(bit_reader->buffer + header_start,
           bit_reader->byte_position - header_start);

  uint32_t stored_crc = 0;
  if (!read_bits(bit_reader, 8, &stored_crc))
    return false;
  if (computed_crc != stored_crc) {
    BR_LOG_ERROR("Frame header CRC-8 mismatch: computed 0x%02X, stored 0x%02X",
                 computed_crc, stored_crc);
    return false;
  }

  out_header->block_size = block_size;
  out_header->sample_rate = sample_rate;
  out_header->bits_per_sample = bits_per_sample;
  return true;
}

static bool decode_frame(BitReader *bit_reader, const StreamInfo *stream_info,
                         int32_t *samples, uint32_t *out_block_size) {
  size_t frame_start = bit_reader->byte_position;

  FrameHeader header;
  if (!parse_frame_header(bit_reader, stream_info, &header))
    return false;

  if (header.bits_per_sample != stream_info->bits_per_sample) {
    BR_LOG_ERROR("Frame bit depth %u does not match the stream's %u",
                 header.bits_per_sample, stream_info->bits_per_sample);
    return false;
  }

  if (header.sample_rate != stream_info->sample_rate) {
    BR_LOG_ERROR("Frame sample rate %u does not match the stream's %u",
                 header.sample_rate, stream_info->sample_rate);
    return false;
  }

  // samples[] is sized for the stream's largest block, so anything bigger
  // would run off the end of it.
  if (header.block_size > stream_info->max_block_size) {
    BR_LOG_ERROR("Frame block size %u exceeds the stream's maximum of %u",
                 header.block_size, stream_info->max_block_size);
    return false;
  }

  if (!decode_subframe(bit_reader, header.block_size, header.bits_per_sample,
                       samples))
    return false;

  align_to_byte(bit_reader);

  uint16_t computed_crc = crc16(bit_reader->buffer + frame_start,
                                bit_reader->byte_position - frame_start);

  uint32_t stored_crc = 0;
  if (!read_bits(bit_reader, 16, &stored_crc))
    return false;
  if (computed_crc != stored_crc) {
    BR_LOG_ERROR("Frame CRC-16 mismatch: computed 0x%04X, stored 0x%04X",
                 computed_crc, stored_crc);
    return false;
  }

  *out_block_size = header.block_size;
  return true;
}

// --- PUBLIC API ---

BrSound *br_flac_load(const uint8_t *data, size_t size) {
  BrSound *sound = NULL;
  int32_t *block = NULL;

  assert(br_flac_can_load(data, size));

  // br_flac_can_load() matched the signature, so decoding starts after it.
  BitReader bit_reader = {
      .buffer = data + sizeof(FLAC_SIGNATURE),
      .size = size - sizeof(FLAC_SIGNATURE),
      .byte_position = 0,
      .bit_position = 0,
  };

  StreamInfo stream_info;
  bool is_last_block = false;
  if (!parse_streaminfo(&bit_reader, &stream_info, &is_last_block)) {
    BR_LOG_ERROR("Failed to parse STREAMINFO block");
    goto error;
  }

  if (!skip_remaining_metadata(&bit_reader, is_last_block)) {
    BR_LOG_ERROR("Failed to walk metadata blocks");
    goto error;
  }

  if (stream_info.total_samples == 0) {
    BR_LOG_ERROR("Unsupported FLAC: stream length is not known up front");
    goto error;
  }

  if (stream_info.total_samples > UINT32_MAX) {
    BR_LOG_ERROR("Unsupported FLAC: stream is too long (%llu samples)",
                 (unsigned long long)stream_info.total_samples);
    goto error;
  }

  if (stream_info.max_block_size == 0) {
    BR_LOG_ERROR("Invalid FLAC: maximum block size is 0");
    goto error;
  }

  // The output buffer is sized from a number read out of the file, so check
  // it against what the remaining bytes could actually encode first. The
  // densest possible frame is a minimal header, a CONSTANT subframe and the
  // two CRCs; FLAC_MIN_FRAME_SIZE stays under that to keep the bound loose
  // enough that no real stream trips it.
  uint64_t remaining_bytes = bit_reader.size - bit_reader.byte_position;
  uint64_t max_possible_samples =
      (remaining_bytes / FLAC_MIN_FRAME_SIZE + 1) * stream_info.max_block_size;
  if (stream_info.total_samples > max_possible_samples) {
    BR_LOG_ERROR("Invalid FLAC: claims %llu samples, more than the remaining "
                 "%llu bytes can encode",
                 (unsigned long long)stream_info.total_samples,
                 (unsigned long long)remaining_bytes);
    goto error;
  }

  sound = malloc(sizeof(BrSound));
  if (!sound) {
    BR_LOG_ERROR("Failed to allocate sound");
    goto error;
  }

  sound->size = (uint32_t)stream_info.total_samples;
  sound->data = malloc(sound->size);
  if (!sound->data) {
    BR_LOG_ERROR("Failed to allocate sound data");
    goto error;
  }

  block = malloc(stream_info.max_block_size * sizeof(int32_t));
  if (!block) {
    BR_LOG_ERROR("Failed to allocate frame sample buffer");
    goto error;
  }

  uint32_t decoded = 0;
  while (decoded < sound->size) {
    uint32_t block_size = 0;
    if (!decode_frame(&bit_reader, &stream_info, block, &block_size)) {
      BR_LOG_ERROR("Failed to decode frame at sample %u", decoded);
      goto error;
    }

    if (block_size > sound->size - decoded) {
      BR_LOG_WARN("Frame holds %u samples but only %u remain, truncating",
                  block_size, sound->size - decoded);
      block_size = sound->size - decoded;
    }

    // FLAC stores samples signed; playback expects unsigned 8-bit.
    for (uint32_t i = 0; i < block_size; i++)
      sound->data[decoded + i] = (uint8_t)(block[i] + 128);

    decoded += block_size;
  }

  free(block);
  BR_LOG_DEBUG("Decoded %u FLAC samples", decoded);
  return sound;

error:
  if (block)
    free(block);
  if (sound) {
    if (sound->data)
      free(sound->data);
    free(sound);
  }
  return NULL;
}
