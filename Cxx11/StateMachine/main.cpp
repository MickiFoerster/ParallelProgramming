#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
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
class StateMachine {
  enum class state {
    disconnected = 0x100,
    connecting,
    open,
    closing,
    closed,
  };

  state currentState = StateMachine::state::disconnected;

  std::unique_ptr<std::thread> tid;
  std::atomic<bool> terminateThread;
  /***************** Transition disconnected -> connecting ******************/
  std::condition_variable condDisconnectedConnecting;
  std::mutex mtxDisconnectedConnecting;
  bool transitionDisconnectedConnecting = false;
  void waitForTransitionDisconnectedConnecting();
  /**************************************************************************/
public:
  StateMachine();
  ~StateMachine();
  void setTransitionDisconnectedConnecting();
  void setTransitionConnectingOpen(short val);
  void setTransitionOpenClosing();
  void setTransitionClosingClosed();
  friend void statemachineTask(StateMachine *sm);
  friend std::ostream& operator<<(std::ostream& os, StateMachine::state s);
};

int main() {
  StateMachine sm;
  std::cerr << "Start to connect ...\n";
  sm.setTransitionDisconnectedConnecting();
  sleep(3);
  return 0;
}

std::ostream& operator<<(std::ostream& os, StateMachine::state s) {
  switch(s) {
      case StateMachine::state::disconnected:
        os << "disconnected";
        break;
      case StateMachine::state::connecting:
        os << "connecting";
        break;
      case StateMachine::state::open:
        os << "open";
        break;
      case StateMachine::state::closing:
        os << "closing";
        break;
      case StateMachine::state::closed:
        os << "closed";
        break;
  }
  return os;
}

void statemachineTask(StateMachine *sm) {
  for (;;) {
    if (sm->terminateThread.load(std::memory_order_acquire)) {
      break;
    }
    std::cerr << "Current state: " << sm->currentState << "\n";
    switch(sm->currentState) {
      case StateMachine::state::disconnected:
        sm->waitForTransitionDisconnectedConnecting();
        sm->currentState = StateMachine::state::connecting;
        break;
      case StateMachine::state::connecting:
        break;
      case StateMachine::state::open:
        break;
      case StateMachine::state::closing:
        break;
      case StateMachine::state::closed:
        break;
    } 
  }
}

StateMachine::~StateMachine() {
  std::cerr << "Terminate state machine ...\n";
  terminateThread.store(true, std::memory_order_release);
  tid->join();
}

StateMachine::StateMachine() {
  terminateThread.store(false, std::memory_order_release);
  tid = std::unique_ptr<std::thread>{new std::thread{statemachineTask, this}};
}

void StateMachine::setTransitionDisconnectedConnecting() {
  mtxDisconnectedConnecting.lock();
  transitionDisconnectedConnecting = true;
  mtxDisconnectedConnecting.unlock();
  condDisconnectedConnecting.notify_one();
}

void StateMachine::waitForTransitionDisconnectedConnecting() {
  std::unique_lock<std::mutex> uniqlock{mtxDisconnectedConnecting};
  std::cerr << __FUNCTION__ << ": wait for transition disconnect -> connecting\n";
  condDisconnectedConnecting.wait(
      uniqlock, [&] { return transitionDisconnectedConnecting; });
  std::cerr << __FUNCTION__ << ": disconnect -> connecting is true\n";
}
