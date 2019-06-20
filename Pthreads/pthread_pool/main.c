#include <stdio.h>
#include <pthread.h>
#include "pthread_pool.h"

void task(void* arg) {
  fprintf(stderr, "0x%0lX: Hello here is a task.\n", pthread_self());
}

int main(void)
{
  void* thread_pool = pthread_pool_new();
  if ( thread_pool ) {
    for(int i=0;i<10;++i)
      pthread_pool_insert_task(thread_pool, task, NULL);
    pthread_pool_wait_until_all_tasks_are_done(thread_pool);
    pthread_pool_delete(thread_pool);
  }

  return 0;
}
