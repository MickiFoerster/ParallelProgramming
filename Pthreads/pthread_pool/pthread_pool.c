#include "pthread_pool.h"

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug_print.h"
#include "queue.h"

typedef struct {
  pthread_mutex_t mtx;
  pthread_t* threads;
  void* task_queue;
} pthread_pool_t;

typedef struct { 
  void (*task)(void*);
  void* arg;
} task_t;

static void* worker(void* arg);

size_t POOL_MAX_SIZE = 4;

void* pthread_pool_new(void) {
  pthread_pool_new_with_size(0);
}

void* pthread_pool_new_with_size(size_t sz) {
  if ( sz ) 
    POOL_MAX_SIZE = sz;
  pthread_pool_t* pool = (pthread_pool_t*) malloc(sizeof(pthread_pool_t));
  if (pool) {
    pthread_mutex_init(&pool->mtx, NULL);
    pool->threads = (pthread_t*) malloc(sizeof(pthread_t)*POOL_MAX_SIZE);
    pool->task_queue = queue_new();
    if ( pool->threads == NULL || pool->task_queue == NULL ) {
      if (pool->threads) free(pool->threads);
      if (pool->task_queue) free(pool->task_queue);
      free(pool);
      pool = NULL;
    } else {
      for(int i=0;i<POOL_MAX_SIZE;++i)
        pthread_create(&pool->threads[i], NULL, worker, pool);
    }
  }
  return pool;
}


void pthread_pool_delete(void* pool) {
  if (pool) {
    pthread_pool_t* p = (pthread_pool_t*) pool;
    pthread_mutex_lock(&p->mtx);
    {
      if ( p->threads ) {
        for(int i=0;i<POOL_MAX_SIZE;++i) {
          debug_print("Cancel thread 0x%0lX\n", p->threads[i]);
          pthread_cancel(p->threads[i]);
        }
        free(p->threads);
        p->threads = NULL;
      }
      if ( p->task_queue ) {
        queue_delete(p->task_queue);
        p->task_queue = NULL;
      }
    }
    pthread_mutex_unlock(&p->mtx);
    pthread_mutex_destroy(&p->mtx);
    free(pool);
  }
}

void pthread_pool_insert_task(void* pool, void (*task)(void* arg), void* arg) {
  if ( !pool ) 
    return;
  task_t* t = (task_t*) malloc(sizeof(task_t));
  if ( t ) {
    pthread_pool_t* p = (pthread_pool_t*) pool;
    t->task = task;
    t->arg = arg;
    queue_push(p->task_queue, t);
  }
}

void pthread_pool_wait_until_all_tasks_are_done(void* pool) {
  pthread_pool_t* p = (pthread_pool_t*) pool;
  queue_finish(p->task_queue);
  fprintf(stderr, "Join with all working threads\n");
  for (int i = 0; i < POOL_MAX_SIZE; ++i) pthread_join(p->threads[i], NULL);
}

static void* worker(void* arg) {
  assert(arg);
  pthread_pool_t* pool = (pthread_pool_t*) arg;
  for(;;) {
    task_t* t = (task_t*) queue_pop(pool->task_queue);
    if ( t ) 
      t->task(t->arg);
    else 
      break;
  }
  return NULL;
}

