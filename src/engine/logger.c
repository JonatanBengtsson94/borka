#include "logger.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_MESSAGE_LENGTH 512
#define MESSAGE_QUEUE_SIZE 256

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
static pthread_t logger_thread;
static MessageQueue msg_queue;

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

static void *logger_thread_func(void *arg) {
  LogMessage msg;

  while (queue_pop(&msg_queue, &msg)) {
    struct tm *t = localtime(&msg.timestamp);
    char time_buf[9];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", t);

    fprintf(stdout, "[%s] [%s] %s\n", time_buf, level_strings[msg.level],
            msg.message);

    if (log_file) {
      fprintf(log_file, "[%s] [%s] %s\n", time_buf, level_strings[msg.level],
              msg.message);
      fflush(log_file);
    }
  }

  return NULL;
}

void logger_init(const char *filename) {
  log_file = fopen(filename, "w");
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
