#include <iostream>
#include <thread>
#include <queue>
#include <functional>
using namespace std;

queue<function<void()>> qu;

void ExecuteQueue() {
  while (!qu.empty()) {
    qu.front()();
    qu.pop();
  }
}

int main() {
  for (int i = 0; i < 20; i++) {
    qu.push([]() {
      std::cout << "Butt cheese\n";
    });
  }
  ExecuteQueue();

  system("pause");
}