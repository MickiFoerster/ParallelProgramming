#include <iostream>
#include <thread>

void statemachine() {
  std::cerr << "Hello\n";
}

int main() {
  std::thread t{statemachine};

  t.join();
  return 0;
}
