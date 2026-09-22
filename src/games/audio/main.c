#include "audio/br_audio.h"
#include "borka_log.h"
#include "borka_time.h"
#include "logger/br_logger.h"

// br_play_sound() is non-blocking; give the audio thread time to actually
// play the sound before tearing everything down.
#define PLAYBACK_WAIT_NS 500000000

static void play_asset(const char *filepath) {
  BrSound *sound = br_sound_create(filepath);
  if (!sound) {
    BR_LOG_ERROR("Skipping '%s': failed to load", filepath);
    return;
  }

  br_play_sound(sound);
  sleep(PLAYBACK_WAIT_NS);
  br_sound_destroy(sound);
}

int main() {
  if (!br_logger_init("Audio"))
    return 1;

  if (!br_audio_init()) {
    br_logger_shutdown();
    return 1;
  }

  play_asset("assets/sfx/bounce.wav");
  play_asset("assets/sfx/bounce.flac");

  br_audio_shutdown();
  br_logger_shutdown();
  return 0;
}
