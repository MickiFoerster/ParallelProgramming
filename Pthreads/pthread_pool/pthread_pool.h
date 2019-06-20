#ifndef PTHREAD_POOL_H
#define PTHREAD_POOL_H

#include <stdlib.h>

void* pthread_pool_new(void);
void* pthread_pool_new_with_size(size_t sz);
void pthread_pool_delete(void*);
void pthread_pool_insert_task(void* pool, void (*task)(void* arg), void* arg);
void pthread_pool_wait_until_all_tasks_are_done(void* pool);

#endif
