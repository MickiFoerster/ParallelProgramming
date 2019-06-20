#ifndef QUEUE_H
#define QUEUE_H 

void* queue_new(void); 
void queue_delete(void* queue);
void queue_push(void* queue, void* data);
void* queue_pop(void* queue);
void queue_finish(void* queue);

#endif
