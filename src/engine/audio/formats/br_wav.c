#include "br_wav.h"
#include "pch.h"

#include "borka_log.h"

typedef struct {
  char riff[4];
  uint32_t file_size;
  char wave[4];
  char fmt[4];
  uint32_t fmt_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
  char data[4];
  uint32_t data_size;
} WAVHeader;

// "RIFF" only identifies the container; "WAVE" identifies the form inside it.
bool br_wav_can_load(const uint8_t *data, size_t size) {
  return size >= 12 && memcmp(data, "RIFF", 4) == 0 &&
         memcmp(data + 8, "WAVE", 4) == 0;
}

static bool parse_wav_header(const uint8_t *data, size_t file_size,
                             WAVHeader *header) {
  if (file_size < sizeof(WAVHeader)) {
    BR_LOG_ERROR("Invalid WAV: file to small");
    return false;
  }

  memcpy(header, data, sizeof(WAVHeader));

  // The RIFF and WAVE identifiers were matched by br_wav_can_load().
  if (memcmp(header->fmt, "fmt ", 4) != 0) {
    BR_LOG_ERROR("Invalid WAV: missing fmt chunk");
    return false;
  }

  if (memcmp(header->data, "data", 4) != 0) {
    BR_LOG_ERROR("Invalid WAV: missing data chunk");
    return false;
  }

  if (header->audio_format != 1) {
    BR_LOG_ERROR("Unsupported WAV: not pcm");
    return false;
  }

  if (header->num_channels != BR_AUDIO_CHANNELS) {
    BR_LOG_ERROR("Unsupported WAV: must be mono");
    return false;
  }

  if (header->bits_per_sample != BR_AUDIO_BITS_PER_SAMPLE) {
    BR_LOG_ERROR("Unsupported WAV: must be %d-bit", BR_AUDIO_BITS_PER_SAMPLE);
    return false;
  }

  if (header->sample_rate != BR_AUDIO_SAMPLE_RATE) {
    BR_LOG_ERROR("Unsupported WAV: sample rate is %u, must be %d",
                 header->sample_rate, BR_AUDIO_SAMPLE_RATE);
    return false;
  }

  // The data chunk's declared length comes from the file, so it has to be
  // checked before it is used to size and fill the sample buffer.
  size_t available = file_size - sizeof(WAVHeader);
  if (header->data_size > available) {
    BR_LOG_ERROR("Invalid WAV: data chunk claims %u bytes, but only %zu remain",
                 header->data_size, available);
    return false;
  }

  return true;
}

BrSound *br_wav_load(const uint8_t *data, size_t size) {
  assert(br_wav_can_load(data, size));

  WAVHeader header;
  if (!parse_wav_header(data, size, &header)) {
    BR_LOG_ERROR("Failed to parse WAV header");
    return NULL;
  }

  BrSound *sound = malloc(sizeof(BrSound));
  if (!sound) {
    BR_LOG_ERROR("Failed to allocate sound");
    return NULL;
  }

  sound->size = header.data_size;
  sound->data = malloc(sound->size);
  if (!sound->data) {
    BR_LOG_ERROR("Failed to allocate sound data");
    free(sound);
    return NULL;
  }

  memcpy(sound->data, data + sizeof(WAVHeader), sound->size);
  return sound;
}
