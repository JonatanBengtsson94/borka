#include "br_event.h"
#include "borka_log.h"

#define EVENT_QUEUE_SIZE 32

typedef struct {
  BrEvent events[EVENT_QUEUE_SIZE];
  int head;
  int tail;
  int count;
} EventQueue;

static EventQueue event_queue;

bool br_event_push(const BrEvent *event) {
  if (event_queue.count >= EVENT_QUEUE_SIZE) {
    BR_LOG_ERROR("Window event queue is full, dropped event");
    return false;
  }

  event_queue.events[event_queue.tail] = *event;
  event_queue.tail = (event_queue.tail + 1) % EVENT_QUEUE_SIZE;
  event_queue.count++;
  return true;
}

bool br_event_poll(BrEvent *out_event) {
  if (event_queue.count == 0) {
    return false;
  }

  *out_event = event_queue.events[event_queue.head];
  event_queue.head = (event_queue.head + 1) % EVENT_QUEUE_SIZE;
  event_queue.count--;
  return true;
}
