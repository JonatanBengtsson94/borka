#include "borka_audio.h"
#include "pch.h"

#include "borka_log.h"
#include "br_audio.h"
#include "formats/br_flac.h"
#include "formats/br_wav.h"
#include "io/br_io.h"

static void cleanup(BrSound *sound) {
  if (sound) {
    if (sound->data)
      free(sound->data);
    free(sound);
  }
}

void br_sound_destroy(BrSound *sound) {
  // The mixer holds raw pointers to sounds, so any voice still playing this
  // one has to be stopped before the samples go away.
  br_stop_sound(sound);
  cleanup(sound);
}

BrSound *br_sound_create(const char *filepath) {
  uint8_t *file_data = NULL;
  size_t file_size;
  file_data = read_entire_file(filepath, &file_size);
  if (!file_data) {
    BR_LOG_ERROR("Failed to create sound, could not read file: '%s'", filepath);
    return NULL;
  }

  BrSound *sound = NULL;

#ifdef BR_AUDIO_SUPPORT_FLAC
  if (!sound && br_flac_can_load(file_data, file_size)) {
    sound = br_flac_load(file_data, file_size);
  }
#endif

#ifdef BR_AUDIO_SUPPORT_WAV
  if (!sound && br_wav_can_load(file_data, file_size)) {
    sound = br_wav_load(file_data, file_size);
  }
#endif

  free(file_data);

  if (!sound) {
    BR_LOG_ERROR(
        "Failed to create sound, unsupported or invalid audio format: '%s'",
        filepath);
    return NULL;
  }

  BR_LOG_DEBUG("Successfully loaded sound: '%s'", filepath);
  return sound;
}
