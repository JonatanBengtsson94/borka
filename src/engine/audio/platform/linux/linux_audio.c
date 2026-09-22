#include "pch.h"

#include "borka_audio.h"
#include "borka_log.h"
#include <alsa/asoundlib.h>
#include <pthread.h>

// How many sounds can play at the same time.
#define MAX_VOICES 16

// Frames mixed and handed to the device per iteration. Also the period size,
// so writing blocks until the device has consumed roughly this much.
#define PERIOD_FRAMES 512

// Periods the device buffers ahead, to survive scheduling jitter.
#define BUFFER_PERIODS 4

typedef struct {
  BrSound *sound;
  uint32_t position; // Next sample of the sound to mix.
  float volume;
  bool looping;
  bool active;
} Voice;

typedef struct {
  pthread_t thread;
  pthread_mutex_t mutex;
  bool initialized;
  bool shutdown;
  Voice voices[MAX_VOICES];
  snd_pcm_t *pcm;
} AudioThread;

static AudioThread audio_thread;

// --- INTERNAL HELPERS ---

static bool open_pcm() {
  const char *devices[] = {"pipewire", "pulse", "default", "hw:0,0"};

  for (int i = 0; i < 4; i++) {
    if (snd_pcm_open(&audio_thread.pcm, devices[i], SND_PCM_STREAM_PLAYBACK,
                     0) >= 0) {
      BR_LOG_INFO("Audio opened using device: '%s'", devices[i]);
      return true;
    }
    BR_LOG_DEBUG("Failed to open audio deviuce: '%s", devices[i]);
  }

  BR_LOG_ERROR("All audio device targets failed");
  return false;
}

// Sums every active voice into out and advances them. Samples are unsigned
// with 128 as silence, so they are mixed around that midpoint and clamped
// back into range. Must be called with the mutex held.
static void mix_period(uint8_t *out, uint32_t frames) {
  int32_t mixed[PERIOD_FRAMES] = {0};

  for (int i = 0; i < MAX_VOICES; i++) {
    Voice *voice = &audio_thread.voices[i];
    if (!voice->active)
      continue;

    // Looping voices wrap and carry on filling the same period, so the
    // seam does not leave a gap of silence behind.
    uint32_t filled = 0;
    while (filled < frames) {
      uint32_t remaining = voice->sound->size - voice->position;
      uint32_t count = frames - filled;
      if (count > remaining)
        count = remaining;

      for (uint32_t frame = 0; frame < count; frame++) {
        int32_t sample =
            (int32_t)voice->sound->data[voice->position + frame] - 128;
        mixed[filled + frame] += (int32_t)(sample * voice->volume);
      }

      voice->position += count;
      filled += count;

      if (voice->position >= voice->sound->size) {
        if (!voice->looping) {
          voice->active = false;
          break;
        }
        voice->position = 0;
      }
    }
  }

  for (uint32_t frame = 0; frame < frames; frame++) {
    int32_t sample = mixed[frame] + 128;
    if (sample < 0)
      sample = 0;
    else if (sample > 255)
      sample = 255;
    out[frame] = (uint8_t)sample;
  }
}

// --- AUDIO THREAD ---

// The stream runs for as long as the audio system is up, carrying silence
// when nothing is playing. Keeping it running avoids stopping and restarting
// the device around every sound.
static void *audio_thread_func(void *arg) {
  (void)arg;
  uint8_t period[PERIOD_FRAMES];

  while (true) {
    pthread_mutex_lock(&audio_thread.mutex);
    if (audio_thread.shutdown) {
      pthread_mutex_unlock(&audio_thread.mutex);
      break;
    }
    mix_period(period, PERIOD_FRAMES);
    pthread_mutex_unlock(&audio_thread.mutex);

    snd_pcm_sframes_t written =
        snd_pcm_writei(audio_thread.pcm, period, PERIOD_FRAMES);
    if (written < 0) {
      written = snd_pcm_recover(audio_thread.pcm, (int)written, 1);
      if (written < 0) {
        BR_LOG_ERROR("Failed to write to PCM: %s",
                     snd_strerror((int)written));
        break;
      }
    }
  }

  return NULL;
}

// --- PUBLIC API ---

bool br_audio_init() {
  pthread_mutex_init(&audio_thread.mutex, NULL);
  audio_thread.shutdown = false;
  memset(audio_thread.voices, 0, sizeof(audio_thread.voices));

  snd_pcm_hw_params_t *params;

  if (!open_pcm()) {
    BR_LOG_ERROR("Failed to open pcm");
    return false;
  }

  if (snd_pcm_hw_params_malloc(&params) < 0) {
    BR_LOG_ERROR("Failed to allocate hw params");
    goto error;
  }

  snd_pcm_hw_params_any(audio_thread.pcm, params);

  if (snd_pcm_hw_params_set_access(audio_thread.pcm, params,
                                   SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
    BR_LOG_ERROR("Failed to set interleaved mode");
    goto error;
  }

  if (snd_pcm_hw_params_set_format(audio_thread.pcm, params,
                                   SND_PCM_FORMAT_U8) < 0) {
    BR_LOG_ERROR("Failed to set format");
    goto error;
  }

  if (snd_pcm_hw_params_set_channels(audio_thread.pcm, params,
                                     BR_AUDIO_CHANNELS) < 0) {
    BR_LOG_ERROR("Failed to set channels number");
    goto error;
  }

  if (snd_pcm_hw_params_set_rate(audio_thread.pcm, params,
                                 BR_AUDIO_SAMPLE_RATE, 0) < 0) {
    BR_LOG_ERROR("Failed to set rate");
    goto error;
  }

  snd_pcm_uframes_t period_size = PERIOD_FRAMES;
  if (snd_pcm_hw_params_set_period_size_near(audio_thread.pcm, params,
                                             &period_size, 0) < 0) {
    BR_LOG_ERROR("Failed to set period size");
    goto error;
  }

  snd_pcm_uframes_t buffer_size = PERIOD_FRAMES * BUFFER_PERIODS;
  if (snd_pcm_hw_params_set_buffer_size_near(audio_thread.pcm, params,
                                             &buffer_size) < 0) {
    BR_LOG_ERROR("Failed to set buffer size");
    goto error;
  }

  if (snd_pcm_hw_params(audio_thread.pcm, params) < 0) {
    BR_LOG_ERROR("Failed to install hw params");
    goto error;
  }

  BR_LOG_DEBUG("Audio buffer: %lu frames in periods of %lu",
               (unsigned long)buffer_size, (unsigned long)period_size);

  if (snd_pcm_prepare(audio_thread.pcm) < 0) {
    BR_LOG_ERROR("Failed to prepare pcm");
    goto error;
  }

  if (pthread_create(&audio_thread.thread, NULL, audio_thread_func, NULL) !=
      0) {
    BR_LOG_ERROR("Failed to create audio thread");
    goto error;
  }

  audio_thread.initialized = true;
  BR_LOG_INFO("Initialized audio system");
  snd_pcm_hw_params_free(params);
  return true;

error:
  snd_pcm_hw_params_free(params);
  return false;
}

void br_audio_shutdown() {
  if (!audio_thread.initialized)
    return;

  pthread_mutex_lock(&audio_thread.mutex);
  audio_thread.shutdown = true;
  pthread_mutex_unlock(&audio_thread.mutex);

  pthread_join(audio_thread.thread, NULL);
  pthread_mutex_destroy(&audio_thread.mutex);

  if (audio_thread.pcm) {
    snd_pcm_drop(audio_thread.pcm);
    snd_pcm_close(audio_thread.pcm);
  }

  snd_config_update_free_global();
  audio_thread.initialized = false;
}

static void start_voice(BrSound *sound, float volume, bool looping) {
  assert(sound && sound->data);

  if (!audio_thread.initialized) {
    BR_LOG_ERROR("Cannot play a sound before the audio system is initialized");
    return;
  }

  // A looping voice would never advance through an empty sound.
  if (sound->size == 0) {
    BR_LOG_ERROR("Refusing to play a sound with no samples");
    return;
  }

  if (volume < 0.0f)
    volume = 0.0f;

  pthread_mutex_lock(&audio_thread.mutex);

  Voice *free_voice = NULL;
  for (int i = 0; i < MAX_VOICES; i++) {
    if (!audio_thread.voices[i].active) {
      free_voice = &audio_thread.voices[i];
      break;
    }
  }

  if (!free_voice) {
    pthread_mutex_unlock(&audio_thread.mutex);
    BR_LOG_WARN("All %d voices are in use, dropping sound", MAX_VOICES);
    return;
  }

  free_voice->sound = sound;
  free_voice->position = 0;
  free_voice->volume = volume;
  free_voice->looping = looping;
  free_voice->active = true;

  pthread_mutex_unlock(&audio_thread.mutex);
  BR_LOG_TRACE("Playing %s of %u samples at volume %.2f",
               looping ? "loop" : "sound", sound->size, (double)volume);
}

void br_play_sound(BrSound *sound) { start_voice(sound, 1.0f, false); }

void br_play_sound_at_volume(BrSound *sound, float volume) {
  start_voice(sound, volume, false);
}

void br_play_sound_looping(BrSound *sound, float volume) {
  start_voice(sound, volume, true);
}

void br_stop_sound(BrSound *sound) {
  if (!audio_thread.initialized)
    return;

  pthread_mutex_lock(&audio_thread.mutex);
  for (int i = 0; i < MAX_VOICES; i++) {
    if (audio_thread.voices[i].sound == sound)
      audio_thread.voices[i].active = false;
  }
  pthread_mutex_unlock(&audio_thread.mutex);
}
