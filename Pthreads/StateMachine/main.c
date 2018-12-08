#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

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

bool checkTransitionUnconnectedConnecting(void) {
  return true;
}

bool checkTransitionConnectingConnected(void) {
  return true;
}

void* statemachineTask(void *argv) {
  typedef enum {
    undefined = 0,
    unconnected = 0x100,
    connecting,
    connected,
  } state_t;

  state_t currentState = unconnected; 
  for(;;) {
    switch( currentState ) {
      case undefined:
        assert(0);
      case unconnected:
        if ( checkTransitionUnconnectedConnecting() )
          currentState = connecting;
        break;
      case connecting:
        if ( checkTransitionConnectingConnected() )
          currentState = connected;
        else
          currentState = unconnected;
        break;
      case connected:
        printf("connected");
        goto end;
        break;
    }
  }
end:
  return NULL;
}

void stateMachine(void) {
  int rc = pthread_create(&tid, NULL, statemachineTask, NULL);
  if ( rc ) 
    fatal("pthread_create failed");
  rc = pthread_detach(tid);
  if ( rc ) 
    fatal("pthread_detach failed");
}
