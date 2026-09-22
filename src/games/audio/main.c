#include "audio/br_audio.h"
#include "borka_log.h"
#include "borka_time.h"
#include "logger/br_logger.h"

// sleep() puts its whole argument into tv_nsec, which nanosleep rejects at a
// full second, so longer waits have to be made in chunks.
#define SLEEP_CHUNK_NS 50000000

// How often a bounce is triggered over the music.
#define SFX_INTERVAL_SECONDS 1.5

// Both assets are normalised to a peak of 112/127, so these only set the
// balance: the music sits back as a bed and the bounce carries over it. They
// peak at 39 and 78, leaving room for the two to land together without
// clipping.
#define MUSIC_VOLUME 0.35f
#define SFX_VOLUME 0.7f

// Tail added to the wait so the device finishes the last samples.
#define PLAYBACK_TAIL_SECONDS 0.25

static void wait_until(double timestamp) {
  while (br_get_time() < timestamp)
    sleep(SLEEP_CHUNK_NS);
}

static double sound_seconds(const BrSound *sound) {
  // 8bit mono, so the byte count is also the sample count.
  return (double)sound->size / BR_AUDIO_SAMPLE_RATE;
}

int main() {
  if (!br_logger_init("Audio"))
    return 1;

  if (!br_audio_init()) {
    br_logger_shutdown();
    return 1;
  }

  BrSound *music = br_sound_create("assets/music/theme.flac");
  BrSound *bounce = br_sound_create("assets/sfx/bounce.flac");
  if (!music || !bounce) {
    BR_LOG_ERROR("Failed to load audio assets");
    br_audio_shutdown();
    br_logger_shutdown();
    return 1;
  }

  double music_seconds = sound_seconds(music);
  BR_LOG_INFO("Playing %.1f seconds of music, with a bounce every %.1f",
              music_seconds, SFX_INTERVAL_SECONDS);

  double started_at = br_get_time();
  br_play_sound_at_volume(music, MUSIC_VOLUME);

  // Trigger the sfx over the music to show both mixing together.
  for (double at = SFX_INTERVAL_SECONDS; at < music_seconds;
       at += SFX_INTERVAL_SECONDS) {
    wait_until(started_at + at);
    BR_LOG_INFO("Bounce at %.1f seconds", at);
    br_play_sound_at_volume(bounce, SFX_VOLUME);
  }

  wait_until(started_at + music_seconds + PLAYBACK_TAIL_SECONDS);

  br_sound_destroy(bounce);
  br_sound_destroy(music);
  br_audio_shutdown();
  br_logger_shutdown();
  return 0;
}
