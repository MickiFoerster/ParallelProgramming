#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "pthread_pool.h"

void task(void* arg) {
  int waiting_usecs;
  fprintf(stderr, "0x%0lX: Hello here is a task.\n", pthread_self());
  waiting_usecs = rand() % (1000 * 1000);
  fprintf(stderr, "Waiting %d usecs\n", waiting_usecs);
  usleep(waiting_usecs);
  fprintf(stderr, "0x%0lX: Thread done.\n", pthread_self());
}

int main(void) {
  srand(time(NULL));
  void* thread_pool = pthread_pool_new();
  if ( thread_pool ) {
    for(int i=0;i<10;++i)
      pthread_pool_insert_task(thread_pool, task, NULL);
    pthread_pool_wait_until_all_tasks_are_done(thread_pool);
    pthread_pool_delete(thread_pool);
  }

  return 0;
}
