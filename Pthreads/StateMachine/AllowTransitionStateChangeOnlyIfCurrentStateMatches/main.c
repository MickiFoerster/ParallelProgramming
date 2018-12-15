#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*                                         -loop-
 * Statemachine from websockets             \ /send()
 * -> connecting -> < SUCCESS > -- yes --> open
 *                      |                    |
 *                      no                   close()
 *                      |                    |
 *                      V                    V
 *                    closed  <----------- closing
 */

#if 0
#define pthread_mutex_lock(...) do { \
    fprintf(stderr, "LOCK\n"); \
    pthread_mutex_lock(__VA_ARGS__); \
  } while(0)
#define pthread_mutex_unlock(...) do { \
    fprintf(stderr, "UNLOCK\n"); \
    pthread_mutex_unlock(__VA_ARGS__); \
  } while(0)
#endif

void stateMachine(void);
void endStatemachine(void);
void setTransitionDisconnectedConnecting(void);
void setTransitionConnectingOpen(short val);
void setTransitionOpenClosing(void);
void setTransitionClosingClosed(void);

int main(int argc, char *argv[]) {
  int i;
  stateMachine();
  fprintf(stderr, "main sleeps ...\n");
  sleep(2);

  fprintf(stderr, "Start connecting ...\n");
  setTransitionDisconnectedConnecting();

  sleep(2);
  setTransitionConnectingOpen(-1); // Fails

  sleep(2); 
  fprintf(stderr, "Reconnects ...\n");
  setTransitionDisconnectedConnecting();

  sleep(2);
  setTransitionConnectingOpen(1); // succeeds

  for(i=0; i<5; ++i) {
    fprintf(stderr, ".");
    sleep(1);// sending data then close connection
  }
  fprintf(stderr, "\n");
  setTransitionOpenClosing();

  sleep(2); // closing connection
  setTransitionClosingClosed();

  endStatemachine();

  pthread_exit(NULL);
  return 0;
}

void fatal(const char errormsg[]) {
  fprintf(stderr, "%s\n", errormsg);
  exit(1);
}

pthread_t tid;
void endStatemachine(void) {
  if (pthread_cancel(tid)) {
    fprintf(stderr, "error: Thread could not be canceled\n");
  }
}

typedef enum {
  disconnected = 0x100,
  connecting,
  open,
  closing,
  closed,
} state_t;
state_t currentState = disconnected;
pthread_mutex_t mtxCurrentState = PTHREAD_MUTEX_INITIALIZER;

/***************** Transition disconnected -> connecting *********************/
pthread_cond_t condDisconnectedConnecting = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxDisconnectedConnecting = PTHREAD_MUTEX_INITIALIZER;
bool transitionDisconnectedConnecting = false;

void setTransitionDisconnectedConnecting(void) {
  pthread_mutex_lock(&mtxCurrentState);
  if ( currentState == disconnected ) {
    pthread_mutex_unlock(&mtxCurrentState);
    pthread_mutex_lock(&mtxDisconnectedConnecting);
    transitionDisconnectedConnecting = true;
    pthread_mutex_unlock(&mtxDisconnectedConnecting);
    pthread_cond_signal(&condDisconnectedConnecting);
  } else {
    pthread_mutex_unlock(&mtxCurrentState);
  }
}

bool checkTransitionDisconnectedConnecting(void) {
  pthread_mutex_lock(&mtxDisconnectedConnecting);
  while (!transitionDisconnectedConnecting)
    pthread_cond_wait(&condDisconnectedConnecting, &mtxDisconnectedConnecting);
  transitionDisconnectedConnecting = false;
  pthread_mutex_unlock(&mtxDisconnectedConnecting);
  return true;
}
/*****************************************************************************/


/***************** Transition connecting -> open / closed ********************/
pthread_cond_t condConnectingOpen = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxConnectingOpen = PTHREAD_MUTEX_INITIALIZER;
short transitionConnectingOpen = 0;

void setTransitionConnectingOpen(short val) {
  pthread_mutex_lock(&mtxCurrentState);
  if ( currentState == connecting ) {
    pthread_mutex_unlock(&mtxCurrentState);
    pthread_mutex_lock(&mtxConnectingOpen);
    transitionConnectingOpen = val;
    pthread_mutex_unlock(&mtxConnectingOpen);
    pthread_cond_signal(&condConnectingOpen);
  } else {
    pthread_mutex_unlock(&mtxCurrentState);
  }
}

bool checkTransitionConnectingOpen(void) {
  bool transition = false;
  pthread_mutex_lock(&mtxConnectingOpen);
  while (transitionConnectingOpen == 0)
    pthread_cond_wait(&condConnectingOpen, &mtxConnectingOpen);
  transition = (transitionConnectingOpen>0) ? true : false;
  transitionConnectingOpen = 0;
  pthread_mutex_unlock(&mtxConnectingOpen);
  return transition;
}
/*****************************************************************************/


/***************** Transition open -> closing *********************/
pthread_cond_t condOpenClosing = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxOpenClosing = PTHREAD_MUTEX_INITIALIZER;
bool transitionOpenClosing = false;

void setTransitionOpenClosing(void) {
  pthread_mutex_lock(&mtxCurrentState);
  if ( currentState == open ) {
    pthread_mutex_unlock(&mtxCurrentState);
    pthread_mutex_lock(&mtxOpenClosing);
    transitionOpenClosing = true;
    pthread_mutex_unlock(&mtxOpenClosing);
    pthread_cond_signal(&condOpenClosing);
  } else {
    pthread_mutex_unlock(&mtxCurrentState);
  }
}

bool checkTransitionOpenClosing(void) {
  pthread_mutex_lock(&mtxOpenClosing);
  while (!transitionOpenClosing)
    pthread_cond_wait(&condOpenClosing, &mtxOpenClosing);
  transitionOpenClosing = false;
  pthread_mutex_unlock(&mtxOpenClosing);
  return true;
}
/*****************************************************************************/

/***************** Transition closing -> closed ******************************/
pthread_cond_t condClosingClosed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxClosingClosed = PTHREAD_MUTEX_INITIALIZER;
bool transitionClosingClosed = false;

void setTransitionClosingClosed(void) {
  pthread_mutex_lock(&mtxCurrentState);
  if ( currentState == closing ) {
    pthread_mutex_unlock(&mtxCurrentState);
    pthread_mutex_lock(&mtxClosingClosed);
    transitionClosingClosed = true;
    pthread_mutex_unlock(&mtxClosingClosed);
    pthread_cond_signal(&condClosingClosed);
  } else {
    pthread_mutex_unlock(&mtxCurrentState);
  }
}

bool checkTransitionClosingClosed(void) {
  pthread_mutex_lock(&mtxClosingClosed);
  while (!transitionClosingClosed)
    pthread_cond_wait(&condClosingClosed, &mtxClosingClosed);
  transitionClosingClosed = false;
  pthread_mutex_unlock(&mtxClosingClosed);
  return true;
}
/*****************************************************************************/

void *statemachineTask(void *argv) {
  for (;;) {
    pthread_mutex_lock(&mtxCurrentState);
    switch (currentState) {
    case disconnected:
      pthread_mutex_unlock(&mtxCurrentState);
      fprintf(stderr, "disconnected\n");
      checkTransitionDisconnectedConnecting();
      pthread_mutex_lock(&mtxCurrentState);
      currentState = connecting;
      pthread_mutex_unlock(&mtxCurrentState);
      break;
    case connecting:
      pthread_mutex_unlock(&mtxCurrentState);
      fprintf(stderr, "connecting\n");
      if (checkTransitionConnectingOpen()) {
        pthread_mutex_lock(&mtxCurrentState);
        currentState = open;
        pthread_mutex_unlock(&mtxCurrentState);
      }
      else {
        pthread_mutex_lock(&mtxCurrentState);
        currentState = closed;
        pthread_mutex_unlock(&mtxCurrentState);
      }
      break;
    case open:
      pthread_mutex_unlock(&mtxCurrentState);
      fprintf(stderr, "open\n");
      checkTransitionOpenClosing();
      pthread_mutex_lock(&mtxCurrentState);
      currentState = closing;
      pthread_mutex_unlock(&mtxCurrentState);
      break;
    case closing:
      pthread_mutex_unlock(&mtxCurrentState);
      fprintf(stderr, "closing\n");
      checkTransitionClosingClosed();
      pthread_mutex_lock(&mtxCurrentState);
      currentState = closed;
      pthread_mutex_unlock(&mtxCurrentState);
      break;
    case closed:
      pthread_mutex_unlock(&mtxCurrentState);
      fprintf(stderr, "closed\n");
      pthread_mutex_lock(&mtxCurrentState);
      currentState = disconnected;
      pthread_mutex_unlock(&mtxCurrentState);
    }
  }
  return NULL;
}

void stateMachine(void) {
  int rc = pthread_create(&tid, NULL, statemachineTask, NULL);
  if (rc)
    fatal("pthread_create failed");
  rc = pthread_detach(tid);
  if (rc)
    fatal("pthread_detach failed");
}
