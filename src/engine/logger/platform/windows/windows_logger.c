#include "pch.h"

#include "logger/br_logger.h"
#include <stdarg.h>
#include <sys/stat.h>

#define WIN32_LEAN_AND_MEAN
#include <direct.h>
#include <windows.h>
#define mkdir_p(path) _mkdir(path)

typedef struct {
  LogMessage message[MESSAGE_QUEUE_SIZE];
  int head;
  int tail;
  int count;
  CRITICAL_SECTION lock;
  CONDITION_VARIABLE not_empty;
  CONDITION_VARIABLE not_full;
  bool shutdown;
} MessageQueue;

static FILE *log_file = NULL;
static const char *level_strings[] = {"TRACE", "DEBUG", "INFO",
                                      "WARN",  "ERROR", "FATAL"};
static char current_log_path[MAX_LOG_FILE_PATH_SIZE];
static size_t bytes_written = 0;
static HANDLE logger_thread;
static MessageQueue msg_queue;

// --- FILE HANDLING ---

static void get_log_dir(char *buffer, size_t size, const char *game_name) {
  const char *appdata = getenv("APPDATA");
  if (appdata) {
    snprintf(buffer, size, "%s/%s/logs", appdata, game_name);
  } else {
    snprintf(buffer, size, ".logs");
  }
}

static int create_dir_r(const char *path) {
  char tmp[MAX_LOG_FILE_PATH_SIZE];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", path);
  len = strlen(tmp);

  if (len > 0 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
    tmp[len - 1] = '\0';
  }

  for (p = tmp + 1; *p; p++) {
    if (*p == '/' || *p == '\\') {
      *p = '\0';
      mkdir_p(tmp);
      *p = '/';
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

// --- QUEUE OPERATIONS ---

static void queue_init(MessageQueue *q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
  q->shutdown = false;
  InitializeCriticalSection(&q->lock);
  InitializeConditionVariable(&q->not_empty);
  InitializeConditionVariable(&q->not_full);
}

static void queue_destroy(MessageQueue *q) {
  DeleteCriticalSection(&q->lock);
}

static bool queue_push(MessageQueue *q, const LogMessage *msg) {
  EnterCriticalSection(&q->lock);

  while (q->count == MESSAGE_QUEUE_SIZE && !q->shutdown) {
    SleepConditionVariableCS(&q->not_full, &q->lock, INFINITE);
  }

  if (q->shutdown) {
    LeaveCriticalSection(&q->lock);
    return false;
  }

  q->message[q->tail] = *msg;
  q->tail = (q->tail + 1) % MESSAGE_QUEUE_SIZE;
  q->count++;

  WakeConditionVariable(&q->not_empty);
  LeaveCriticalSection(&q->lock);
  return true;
}

static bool queue_pop(MessageQueue *q, LogMessage *msg) {
  EnterCriticalSection(&q->lock);

  while (q->count == 0 && !q->shutdown) {
    SleepConditionVariableCS(&q->not_empty, &q->lock, INFINITE);
  }

  if (q->count == 0 && q->shutdown) {
    LeaveCriticalSection(&q->lock);
    return false;
  }

  *msg = q->message[q->head];
  q->head = (q->head + 1) % MESSAGE_QUEUE_SIZE;
  q->count--;

  WakeConditionVariable(&q->not_full);
  LeaveCriticalSection(&q->lock);
  return true;
}

// --- LOGGER THREAD ---

static DWORD WINAPI logger_thread_func(LPVOID arg) {
  (void)arg;

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

  return 0;
}

// --- PUBLIC API ---

bool br_logger_init(const char *game_name) {
  char log_dir[MAX_LOG_FILE_PATH_SIZE];
  get_log_dir(log_dir, sizeof(log_dir), game_name);
  create_dir_r(log_dir);

  int ret = snprintf(current_log_path, sizeof(current_log_path), "%s%c%s.log",
                     log_dir, '/', game_name);
  if (ret < 0 || (size_t)ret >= sizeof(current_log_path)) {
    fprintf(stderr, "Failed to format log path correctly\n");
    return false;
  }

  struct _stat st;
  if (_stat(current_log_path, &st) == 0) {
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
    fprintf(stderr, "Failed to open log file: %s\n", current_log_path);
  }

  queue_init(&msg_queue);

  logger_thread = CreateThread(NULL, 0, logger_thread_func, NULL, 0, NULL);
  if (!logger_thread) {
    fprintf(stderr, "Failed to create logger thread\n");
    if (log_file) {
      fclose(log_file);
      log_file = NULL;
    }
    queue_destroy(&msg_queue);
    return false;
  }

  return true;
}

void br_logger_shutdown(void) {
  EnterCriticalSection(&msg_queue.lock);
  msg_queue.shutdown = true;
  WakeAllConditionVariable(&msg_queue.not_empty);
  WakeAllConditionVariable(&msg_queue.not_full);
  LeaveCriticalSection(&msg_queue.lock);

  WaitForSingleObject(logger_thread, INFINITE);
  CloseHandle(logger_thread);

  queue_destroy(&msg_queue);

  if (log_file) {
    fclose(log_file);
    log_file = NULL;
  }
}

void _br_logger_message(BrLogLevel level, const char *format, ...) {
  LogMessage msg;
  msg.level = level;
  msg.timestamp = time(NULL);

  va_list args;
  va_start(args, format);
  vsnprintf(msg.message, MAX_MESSAGE_LENGTH, format, args);
  va_end(args);

  msg.message[MAX_MESSAGE_LENGTH - 1] = '\0';
  queue_push(&msg_queue, &msg);
}
