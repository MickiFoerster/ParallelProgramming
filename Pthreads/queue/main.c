#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "queue.h"

const unsigned sz = 10000000; /* 10 000 000 */
int* arr;

void* producer(void* q) {
  arr = (int*) malloc(sizeof(int)*sz);
  int i;
  for(i=0;i<sz;++i)
    arr[i] = i+1;
  for(i=0;i<sz;++i)
    queue_push(q, &arr[i]);
  queue_finish(q);
  return NULL;
}

void* consumer(void* q) {
  int i, *j;
  for(i=0;i<sz;++i) {
    j = (int*) queue_pop(q);
    if(j) printf("%d\n", *j);
  }
  free(arr);
  return NULL;
}

int main() {
  void* q = queue_new(); assert(q);

  pthread_t producer_tid, consumer_tid;
  pthread_create(&consumer_tid, NULL, consumer, q);
  pthread_create(&producer_tid, NULL, producer, q);
  pthread_join(consumer_tid, NULL);
  pthread_join(producer_tid, NULL);

  queue_delete(q);
  pthread_exit(NULL);

  return 0;
}
