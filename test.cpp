#include "array_lock_free_queue.h"
#include <cassert>
#include <iostream>
#include <thread>
using std::cout;

void test_push() {
  ArrayLockFreeQueue<int, 3> q;
  q.push(1);
  assert(q.writeIndex == 1);
  assert(q.maximumReadIndex == 1);

  q.push(3);
  assert(q.writeIndex == 2);
  assert(q.maximumReadIndex == 2);

  q.push(5);
  assert(q.writeIndex == 3);
  assert(q.maximumReadIndex == 3);

  assert(!q.push(7));
}

void test_pop() {
  ArrayLockFreeQueue<int, 3> q;
  q.push(1);
  q.push(3);
  q.push(5);
  int item = 0;

  q.pop(item);
  assert(q.readIndex == 1);
  assert(item == 1);

  q.pop(item);
  assert(q.readIndex == 2);
  assert(item == 3);

  q.pop(item);
  assert(q.readIndex == 3);
  assert(item == 5);

  assert(!q.pop(item));
}

void test_in_searil() {
  test_push();
  test_pop();
}

void test_in_multithreads() {
  ArrayLockFreeQueue<int, 31> q;

  constexpr int size = 10;
  std::thread writer1([&]() {
    for (int i = 1; i < size + 1; i += 2) {
      q.push(i);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  std::thread writer2([&]() {
    for (int i = 2; i < size + 2; i += 2) {
      q.push(i);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  std::thread reader([&]() {
    // pop is not blocking, so wait the writer push item by sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int item = 0;
    bool succed = q.pop(item);
    while (succed == true) {
      /* assert(*item == count); */
      std::cout << item << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      succed = q.pop(item);
    }
  });

  writer1.join();
  writer2.join();
  reader.join();
}
int main() {
  test_in_multithreads();
  return 0;
}
