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

/***************** Transition disconnected -> connecting *********************/
pthread_cond_t condDisconnectedConnecting = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxDisconnectedConnecting = PTHREAD_MUTEX_INITIALIZER;
bool transitionDisconnectedConnecting = false;

void setTransitionDisconnectedConnecting(void) {
  pthread_mutex_lock(&mtxDisconnectedConnecting);
  transitionDisconnectedConnecting = true;
  pthread_mutex_unlock(&mtxDisconnectedConnecting);
  pthread_cond_signal(&condDisconnectedConnecting);
}

static bool checkTransitionDisconnectedConnecting(void) {
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
  pthread_mutex_lock(&mtxConnectingOpen);
  transitionConnectingOpen = val;
  pthread_mutex_unlock(&mtxConnectingOpen);
  pthread_cond_signal(&condConnectingOpen);
}

static bool checkTransitionConnectingOpen(void) {
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
  pthread_mutex_lock(&mtxOpenClosing);
  transitionOpenClosing = true;
  pthread_mutex_unlock(&mtxOpenClosing);
  pthread_cond_signal(&condOpenClosing);
}

static bool checkTransitionOpenClosing(void) {
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
  pthread_mutex_lock(&mtxClosingClosed);
  transitionClosingClosed = true;
  pthread_mutex_unlock(&mtxClosingClosed);
  pthread_cond_signal(&condClosingClosed);
}

static bool checkTransitionClosingClosed(void) {
  pthread_mutex_lock(&mtxClosingClosed);
  while (!transitionClosingClosed)
    pthread_cond_wait(&condClosingClosed, &mtxClosingClosed);
  transitionClosingClosed = false;
  pthread_mutex_unlock(&mtxClosingClosed);
  return true;
}
/*****************************************************************************/


static void *statemachineTask(void *argv) {
  typedef enum {
    disconnected = 0x100,
    connecting,
    open,
    closing,
    closed,
  } state_t;

  state_t currentState = disconnected;
  for (;;) {
    switch (currentState) {
    case disconnected:
      fprintf(stderr, "disconnected\n");
      checkTransitionDisconnectedConnecting();
      currentState = connecting;
      break;
    case connecting:
      fprintf(stderr, "connecting\n");
      if (checkTransitionConnectingOpen())
        currentState = open;
      else
        currentState = closed;
      break;
    case open:
      fprintf(stderr, "open\n");
      checkTransitionOpenClosing();
      currentState = closing;
      break;
    case closing:
      fprintf(stderr, "closing\n");
      checkTransitionClosingClosed();
      currentState = closed;
      break;
    case closed:
      fprintf(stderr, "closed\n");
      currentState = disconnected;
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
