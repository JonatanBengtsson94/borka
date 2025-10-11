#include "logger.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifndef GAME_NAME
#define GAME_NAME "defaultgame"
#endif
#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#define mkdir_p(path) mkdir(path, 0755)
#define PATH_SEP '/'
#else
#error "Unsupported platform."
#endif

#define MAX_MESSAGE_LENGTH 512
#define MESSAGE_QUEUE_SIZE 256
#define MAX_LOG_SIZE (10 * 1024 * 1024)
#define MAX_LOG_FILES 5
#define MAX_LOG_FILE_PATH_SIZE 512

typedef struct {
  LogLevel level;
  char message[MAX_MESSAGE_LENGTH];
  time_t timestamp;
} LogMessage;

typedef struct {
  LogMessage message[MESSAGE_QUEUE_SIZE];
  int head;
  int tail;
  int count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
  bool shutdown;
} MessageQueue;

static FILE *log_file = NULL;
static const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR",
                                      "FATAL"};
static char current_log_path[MAX_LOG_FILE_PATH_SIZE];
static size_t bytes_written = 0;
static pthread_t logger_thread;
static MessageQueue msg_queue;

// --- File handling ---

static void get_log_dir(char *buffer, size_t size) {
#ifdef __linux__
  const char *home = getenv("HOME");
  if (home) {
    snprintf(buffer, size, "%s/.local/share/%s/logs", home, GAME_NAME);
  } else {
    snprintf(buffer, size, ".logs");
  }
#else
#error "Unsupported platform"
#endif
}

static int create_dir_r(const char *path) {
  char tmp[MAX_LOG_FILE_PATH_SIZE];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", path);
  len = strlen(tmp);

  if (len > 0 && tmp[len - 1] == PATH_SEP) {
    tmp[len - 1] = '\0';
  }

  for (p = tmp + 1; *p; p++) {
    if (*p == PATH_SEP) {
      *p = '\0';
      mkdir_p(tmp);
      *p = PATH_SEP;
    }
  }

  return mkdir_p(tmp);
}

static void logger_rotate(void) {
  if (!log_file)
    return;

  fclose(log_file);
  log_file = NULL;

  char old_path[MAX_LOG_FILE_PATH_SIZE];
  int ret = snprintf(old_path, sizeof(old_path), "%s.%d", current_log_path,
                     MAX_LOG_FILES);
  if (ret < 0 || (size_t)ret >= sizeof(old_path)) {
    fprintf(stderr, "Failed to format log path correctly\n");
    return;
  }
  remove(old_path);

  // Shift existing log files: game.log.N -> game.log.N+1
  for (int i = MAX_LOG_FILES - 1; i >= 1; i--) {
    char src[MAX_LOG_FILE_PATH_SIZE], dst[MAX_LOG_FILE_PATH_SIZE];

    ret = snprintf(src, sizeof(src), "%s.%d", current_log_path, i);
    if (ret < 0 || (size_t)ret >= sizeof(src)) {
      fprintf(stderr, "Failed to format log path correctly\n");
      return;
    }

    ret = snprintf(dst, sizeof(dst), "%s.%d", current_log_path, i + 1);
    if (ret < 0 || (size_t)ret >= sizeof(dst)) {
      fprintf(stderr, "Failed to format log path correctly\n");
      return;
    }

    rename(src, dst);
  }

  // Rename current log: game.log -> game.log.1
  char backup[MAX_LOG_FILE_PATH_SIZE];

  ret = snprintf(backup, sizeof(backup), "%s.1", current_log_path);
  if (ret < 0 || (size_t)ret >= sizeof(backup)) {
    fprintf(stderr, "Failed to format log path correctly\n");
    return;
  }

  rename(current_log_path, backup);

  log_file = fopen(current_log_path, "w");
  if (!log_file) {
    fprintf(stderr, "Failed to reopen log file after rotation: %s\n",
            current_log_path);
  }
  bytes_written = 0;
}

// --- Queue operations ---

static void queue_init(MessageQueue *q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
  q->shutdown = false;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->not_empty, NULL);
  pthread_cond_init(&q->not_full, NULL);
}

static void queue_destroy(MessageQueue *q) {
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->not_empty);
  pthread_cond_destroy(&q->not_full);
}

static bool queue_push(MessageQueue *q, const LogMessage *msg) {
  pthread_mutex_lock(&q->mutex);

  while (q->count == MESSAGE_QUEUE_SIZE && !q->shutdown) {
    pthread_cond_wait(&q->not_full, &q->mutex);
  }

  if (q->shutdown) {
    pthread_mutex_unlock(&q->mutex);
    return false;
  }

  q->message[q->tail] = *msg;
  q->tail = (q->tail + 1) % MESSAGE_QUEUE_SIZE;
  q->count++;

  pthread_cond_signal(&q->not_empty);
  pthread_mutex_unlock(&q->mutex);
  return true;
}

static bool queue_pop(MessageQueue *q, LogMessage *msg) {
  pthread_mutex_lock(&q->mutex);

  while (q->count == 0 && !q->shutdown) {
    pthread_cond_wait(&q->not_empty, &q->mutex);
  }

  if (q->count == 0 && q->shutdown) {
    pthread_mutex_unlock(&q->mutex);
    return false;
  }

  *msg = q->message[q->head];
  q->head = (q->head + 1) % MESSAGE_QUEUE_SIZE;
  q->count--;

  pthread_cond_signal(&q->not_full);
  pthread_mutex_unlock(&q->mutex);
  return true;
}

// --- Logger thread ---

static void *logger_thread_func(void *arg) {
  LogMessage msg;

  while (queue_pop(&msg_queue, &msg)) {
    struct tm *t = localtime(&msg.timestamp);
    char time_buf[9];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", t);

    char log_line[MAX_MESSAGE_LENGTH];
    int len = snprintf(log_line, sizeof(log_line), "[%s] [%s] %s\n", time_buf,
                       level_strings[msg.level], msg.message);

    fprintf(stdout, "%s", log_line);

    if (log_file) {
      fprintf(log_file, "%s", log_line);
      fflush(log_file);

      bytes_written += len;

      if (bytes_written > MAX_LOG_SIZE) {
        logger_rotate();
      }
    }
  }

  return NULL;
}

// --- Public API ---

void logger_init(const char *filename) {
  char log_dir[MAX_LOG_FILE_PATH_SIZE];
  get_log_dir(log_dir, sizeof(log_dir));
  create_dir_r(log_dir);

  int ret = snprintf(current_log_path, sizeof(current_log_path), "%s%c%s.log",
                     log_dir, PATH_SEP, GAME_NAME);
  if (ret < 0 || (size_t)ret >= sizeof(current_log_path)) {
    fprintf(stderr, "Failed to format log path correctly\n");
    return;
  }

  struct stat st;
  if (stat(current_log_path, &st) == 0) {
    bytes_written = st.st_size;

    if (bytes_written > MAX_LOG_SIZE) {
      log_file = fopen(current_log_path, "a");
      logger_rotate();
    } else {
      log_file = fopen(current_log_path, "a");
    }
  } else {
    log_file = fopen(current_log_path, "w");
    bytes_written = 0;
  }

  if (!log_file) {
    fprintf(stderr, "Failed to open log file: %s\n", filename);
  }

  queue_init(&msg_queue);

  if (pthread_create(&logger_thread, NULL, logger_thread_func, NULL) != 0) {
    fprintf(stderr, "Failed to create logger thread\n");
    if (log_file) {
      fclose(log_file);
      log_file = NULL;
    }
    queue_destroy(&msg_queue);
  }
}

void logger_shutdown(void) {
  pthread_mutex_lock(&msg_queue.mutex);
  msg_queue.shutdown = true;
  pthread_cond_broadcast(&msg_queue.not_empty);
  pthread_cond_broadcast(&msg_queue.not_full);
  pthread_mutex_unlock(&msg_queue.mutex);
  pthread_join(logger_thread, NULL);

  queue_destroy(&msg_queue);

  if (log_file) {
    fclose(log_file);
    log_file = NULL;
  }
}

void logger_message(LogLevel level, const char *message) {
  LogMessage msg;
  msg.level = level;
  msg.timestamp = time(NULL);

  strncpy(msg.message, message, MAX_MESSAGE_LENGTH - 1);
  msg.message[MAX_MESSAGE_LENGTH - 1] = '\0';

  queue_push(&msg_queue, &msg);
}
