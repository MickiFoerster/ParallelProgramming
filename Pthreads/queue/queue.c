#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <strings.h>
#include <time.h>
#include <pthread.h>

#include "debug_print.h"
#include "queue.h"

size_t QUEUE_MAX_SIZE = 64;

// Element data type
typedef struct {
  void* data;
  struct timespec insertion_time;
} queue_element_t;

// Queue data type 
typedef struct {
  // thread synchronization
  pthread_mutex_t mtx;
  pthread_cond_t insertable;
  pthread_cond_t element_available;
  // content administration
  bool entry_closed;
  bool finished;
  unsigned current_num_of_elements;
  unsigned index_in;
  unsigned index_out;
  // content
  queue_element_t* elements; 
} queue_t;

// Forward declarations
static void init_queue(queue_t* q);
static void deinit_queue(queue_t* q);
static void print_stats(queue_t *q);
static void atexit_handler(void);
static void insert_data(queue_t* queue, void* data);
static void* remove_data(queue_t* queue);

// global data
static size_t num_of_queues = 0;
static pthread_mutex_t sync_access_to_num_of_queues = PTHREAD_MUTEX_INITIALIZER;

void queue_finish(void* queue) {
  if ( queue ) {
    queue_t* q = (queue_t*) queue;
    // Insert dummy element into queue to signal consumers that this 
    // is the end of service.
    queue_push(queue, NULL);
  }
}

void* queue_new_with_size(size_t sz) { 
  if ( sz )
    QUEUE_MAX_SIZE = sz;
  queue_t* q = (queue_t*) malloc(sizeof(queue_t));
  bzero(q, sizeof(queue_t));
  if ( q ) {
    init_queue(q);
    const size_t sz = sizeof(queue_element_t)*QUEUE_MAX_SIZE;
    q->elements = (queue_element_t*) malloc(sz);
    if ( q->elements ) {
      bzero(q->elements, sz);
      pthread_mutex_lock(&sync_access_to_num_of_queues);
      if ( num_of_queues == 0 ) {
        atexit(atexit_handler);
      }
      num_of_queues++;
      pthread_mutex_unlock(&sync_access_to_num_of_queues);
    } else {
      free(q);
    }
  }

  return q;
}

void* queue_new(void) { 
  return queue_new_with_size(0);
}

void queue_delete(void* queue) {
  if ( queue ) {
    queue_t* q = (queue_t*)queue;
    if ( q->elements ) {
      print_stats(q);
      deinit_queue(q);
      assert(q->elements==NULL);
      free(q);
      pthread_mutex_lock(&sync_access_to_num_of_queues);
      num_of_queues--;
      pthread_mutex_unlock(&sync_access_to_num_of_queues);
      debug_print("info: %s(%p) successful\n", __FUNCTION__, (void*)queue);
    } else {
      debug_print("warning: Potential double deallocation of %p\n", queue);
    }
  }
}

void queue_push(void* queue, void* data) {
  if ( queue ) { 
    queue_t* q = (queue_t*) queue;
    pthread_mutex_lock(&q->mtx);
    {
      if ( !q->entry_closed ) { // For customers coming after finish()
        while ( q->index_in == q->index_out &&
                q->current_num_of_elements == QUEUE_MAX_SIZE ) {
          pthread_cond_wait(&q->insertable, &q->mtx);
        }
        insert_data(q, data);
        if ( data == NULL ) // If dummy element then close entry
          q->entry_closed = true;
        pthread_cond_signal(&q->element_available);
      }
    }
    pthread_mutex_unlock(&q->mtx);
  }
}

void* queue_pop(void* queue) {
  void* p = NULL;
  if ( queue ) { 
    queue_t* q = (queue_t*) queue;
    assert(q->elements);
    pthread_mutex_lock(&q->mtx);
    {
      while ( q->index_in == q->index_out &&
              q->current_num_of_elements == 0 &&
              !q->finished ) {
        pthread_cond_wait(&q->element_available, &q->mtx);
      }
      if ( q->current_num_of_elements ) {
        p = remove_data(q);
        if ( p == NULL ) { // Look for signal that indicates end of service
          q->finished = true;
          pthread_cond_broadcast(&q->element_available);
        } else {
          pthread_cond_signal(&q->insertable);
        }
      }
    }
    pthread_mutex_unlock(&q->mtx);
  }
  return p;
}

static void init_queue(queue_t* q) {
  if ( q ) {
    pthread_mutex_init(&q->mtx, NULL);
    pthread_cond_init(&q->insertable, NULL);
    pthread_cond_init(&q->element_available, NULL);
    q->entry_closed = false;
    q->finished = false;
    q->current_num_of_elements = 0;
    q->index_in = 0;
    q->index_out = 0;
    q->elements = NULL;
  }
}

static void deinit_queue(queue_t* q) {
  if ( q ) {
    pthread_mutex_destroy(&q->mtx);
    pthread_cond_destroy(&q->insertable);
    pthread_cond_destroy(&q->element_available);
    if ( q->elements ) {
      free(q->elements);
      q->elements = NULL;
    }
  }
}

static void print_stats(queue_t* q) {
}

static void atexit_handler(void) {
  pthread_mutex_lock(&sync_access_to_num_of_queues);
  if ( num_of_queues != 0 ) {
    debug_print("warning: Not all queues have been deallocated. "
        "Still allocated: %lu.\n", num_of_queues);
  } else {
    debug_print("info: All queues were deallocated.\n");
  }
  pthread_mutex_unlock(&sync_access_to_num_of_queues);
}

static void insert_data(queue_t* queue, void* data) {
  assert(queue);
  assert(queue->elements);
  queue->elements[queue->index_in].data = data;
  queue->index_in = (queue->index_in+1) % QUEUE_MAX_SIZE;
  queue->current_num_of_elements++;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &queue->elements[queue->index_in].insertion_time);
}

static void* remove_data(queue_t* queue) {
  assert(queue);
  assert(queue->elements);
  void* data = queue->elements[queue->index_out].data;
  queue->elements[queue->index_out].data = NULL;
  queue->index_out = (queue->index_out+1) % QUEUE_MAX_SIZE;
  queue->current_num_of_elements--;
  return data;
}

