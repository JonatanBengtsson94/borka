#include "pch.h"

#include "borka_audio.h"
#include "borka_log.h"
#include <alsa/asoundlib.h>
#include <pthread.h>

#define SAMPLE_RATE 22050
#define CHANNELS 1
#define BIT_DEPTH 8

typedef struct {
  pthread_t thread;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool shutdown;
  BrSound *pending_sound;
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

static void play_sound_internal(BrSound *sound) {
  assert(sound && sound->data);

  snd_pcm_prepare(audio_thread.pcm);

  snd_pcm_sframes_t frames =
      snd_pcm_writei(audio_thread.pcm, sound->data, sound->size);
  if (frames < 0) {
    frames = snd_pcm_recover(audio_thread.pcm, frames, 0);
    if (frames < 0) {
      BR_LOG_ERROR("Failed to write to PCM: %s", snd_strerror(frames));
      return;
    }
  }
  BR_LOG_TRACE("Playing sound, Wrote %ld frames", frames);
}

// --- AUDIO THREAD ---

static void *audio_thread_func(void *arg) {
  (void)arg;
  pthread_mutex_lock(&audio_thread.mutex);

  while (!audio_thread.shutdown) {
    pthread_cond_wait(&audio_thread.cond, &audio_thread.mutex);

    if (audio_thread.shutdown)
      break;

    BrSound *sound_to_play = audio_thread.pending_sound;
    audio_thread.pending_sound = NULL;

    pthread_mutex_unlock(&audio_thread.mutex);
    play_sound_internal(sound_to_play);
    pthread_mutex_lock(&audio_thread.mutex);
  }

  pthread_mutex_unlock(&audio_thread.mutex);
  return NULL;
}

// --- PUBLIC API ---

bool br_audio_init() {

  pthread_mutex_init(&audio_thread.mutex, NULL);
  pthread_cond_init(&audio_thread.cond, NULL);
  audio_thread.shutdown = false;
  audio_thread.pending_sound = NULL;

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

  if (snd_pcm_hw_params_set_channels(audio_thread.pcm, params, CHANNELS) < 0) {
    BR_LOG_ERROR("Failed to set channels number");
    goto error;
  }

  if (snd_pcm_hw_params_set_rate(audio_thread.pcm, params, SAMPLE_RATE, 0) <
      0) {
    BR_LOG_ERROR("Failed to set rate");
    goto error;
  }

  if (snd_pcm_hw_params(audio_thread.pcm, params) < 0) {
    BR_LOG_ERROR("Failed to install hw params");
    goto error;
  }

  if (pthread_create(&audio_thread.thread, NULL, audio_thread_func, NULL) !=
      0) {
    BR_LOG_ERROR("Failed to create audio thread");
    goto error;
  }

  BR_LOG_INFO("Initialized audio system");
  snd_pcm_hw_params_free(params);
  return true;

error:
  snd_pcm_hw_params_free(params);
  return false;
}

void br_audio_shutdown() {
  pthread_mutex_lock(&audio_thread.mutex);
  audio_thread.shutdown = true;
  pthread_cond_signal(&audio_thread.cond);
  pthread_mutex_unlock(&audio_thread.mutex);

  pthread_join(audio_thread.thread, NULL);
  pthread_mutex_destroy(&audio_thread.mutex);
  pthread_cond_destroy(&audio_thread.cond);

  if (audio_thread.pcm) {
    snd_pcm_drain(audio_thread.pcm);
    snd_pcm_close(audio_thread.pcm);
  }

  snd_config_update_free_global();
}

void br_play_sound(BrSound *sound) {
  pthread_mutex_lock(&audio_thread.mutex);
  audio_thread.pending_sound = sound;
  pthread_cond_signal(&audio_thread.cond);
  pthread_mutex_unlock(&audio_thread.mutex);
}
