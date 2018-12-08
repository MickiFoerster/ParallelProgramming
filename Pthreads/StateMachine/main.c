#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void stateMachine(void);

int main(int argc, char* argv[])
{
  stateMachine();
  pthread_exit(NULL);
  return 0;
}

pthread_t tid;

void fatal(const char errormsg[]) {
  fprintf(stderr, "%s\n", errormsg);
  exit(1);
}

void* task(void *argv) {
  return NULL;
}

void stateMachine(void) {
  int rc = pthread_create(&tid, NULL, task, NULL);
  if ( rc ) 
    fatal("pthread_create failed");
  rc = pthread_detach(tid);
  if ( rc ) 
    fatal("pthread_detach failed");
}
