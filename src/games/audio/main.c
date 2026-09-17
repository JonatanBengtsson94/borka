#include "audio/br_audio.h"
#include "borka_time.h"
#include "logger/br_logger.h"

int main() {
  if (!br_logger_init("Audio"))
    return 1;

  if (!br_audio_init()) {
    br_logger_shutdown();
    return 1;
  }

  BrSound *sound = br_sound_create("assets/sfx/bounce.wav");
  if (!sound) {
    br_audio_shutdown();
    br_logger_shutdown();
    return 1;
  }

  br_play_sound(sound);

  // br_play_sound() is non-blocking; give the audio thread time to actually
  // play the sound before tearing everything down.
  sleep(500000000);

  br_sound_destroy(sound);
  br_audio_shutdown();
  br_logger_shutdown();
  return 0;
}
