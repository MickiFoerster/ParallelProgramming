#include <iostream>
#include <thread>
#include <atomic>
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
  std::unique_ptr<std::thread> tid;
  std::atomic<bool> terminateThread;
public:
  StateMachine();
  ~StateMachine();
  void endStatemachine(void);
  void setTransitionDisconnectedConnecting(void);
  void setTransitionConnectingOpen(short val);
  void setTransitionOpenClosing(void);
  void setTransitionClosingClosed(void);
  friend void statemachineTask(StateMachine* sm);
};

int main() {
  StateMachine statemachine;
  return 0;
}

void statemachineTask(StateMachine* sm) {
  for(;;) {
    if (sm->terminateThread.load(std::memory_order_acquire)) {
      break;
    }
  }
}

StateMachine::~StateMachine() {
  terminateThread.store(true, std::memory_order_release);
  tid->join();
}

StateMachine::StateMachine() {
  terminateThread.store(false, std::memory_order_release);
  tid = std::unique_ptr<std::thread>{new std::thread{statemachineTask, this}};
}
