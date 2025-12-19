#include "borka_audio.h"
#include "pch.h"

#include "borka_log.h"
#include "br_audio.h"
#include "io/br_io.h"

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

static void cleanup(BrSound *sound) {
  if (sound) {
    if (sound->data)
      free(sound->data);
    free(sound);
  }
}

static bool parse_wav_header(const uint8_t *data, size_t file_size,
                             WAVHeader *header) {
  if (file_size < sizeof(WAVHeader)) {
    BR_LOG_ERROR("Invalid WAV: file to small");
    return false;
  }

  memcpy(header, data, sizeof(WAVHeader));

  if (memcmp(header->riff, "RIFF", 4) != 0) {
    BR_LOG_ERROR("Invalid WAV: missing RIFF");
    return false;
  }

  if (memcmp(header->wave, "WAVE", 4) != 0) {
    BR_LOG_ERROR("Invalid WAV: missing WAVE");
    return false;
  }

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

  if (header->num_channels != 1) {
    BR_LOG_ERROR("Unsupported WAV: must be mono");
    return false;
  }

  if (header->bits_per_sample != 8) {
    BR_LOG_ERROR("Unsupported WAV: must be 8-bit");
    return false;
  }

  return true;
}

void br_sound_destroy(BrSound *sound) { cleanup(sound); }

BrSound *br_sound_create(const char *filepath) {
  uint8_t *file_data = NULL;
  BrSound *sound = NULL;
  size_t file_size;
  file_data = read_entire_file(filepath, &file_size);
  if (!file_data) {
    BR_LOG_ERROR("Failed to create sound, could not read file: '%s'", filepath);
    goto error;
  }

  WAVHeader header;
  if (!parse_wav_header(file_data, file_size, &header)) {
    BR_LOG_ERROR("Failed to parse WAV header: '%s'", filepath);
    goto error;
  }

  sound = malloc(sizeof(BrSound));
  if (!sound) {
    BR_LOG_ERROR("Failed to allocate sound");
    goto error;
  }

  sound->size = header.data_size;
  sound->data = malloc(sound->size);
  if (!sound->data) {
    BR_LOG_ERROR("Failed to allocate sound data");
    goto error;
  }

  memcpy(sound->data, file_data + sizeof(WAVHeader), sound->size);
  free(file_data);

  BR_LOG_DEBUG("Successfully loaded sound: '%s'", filepath);
  return sound;

error:
  if (file_data)
    free(file_data);
  cleanup(sound);
  return NULL;
}
