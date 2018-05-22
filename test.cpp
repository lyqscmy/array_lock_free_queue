#include "array_lock_free_queue.h"
#include <atomic>
#include <cassert>
#include <chrono>
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

  constexpr int size = 100;
  std::atomic<int> pushsum(0);
  std::thread writer1([&]() {
    for (int i = 0; i < size; i += 2) {
      bool succed = false;
      while (succed != true) {
        succed = q.push(i);
      }
      std::atomic_fetch_add(&pushsum, i);
    }
  });

  std::thread writer2([&]() {
    for (int i = 1; i < size; i += 2) {
      bool succed = false;
      while (succed != true) {
        succed = q.push(i);
      }
      std::atomic_fetch_add(&pushsum, i);
    }
  });

  std::atomic<int> popsum(0);
  std::thread reader1([&]() {
    for (int i = 0; i < size / 2; i++) {
      int item = -1;
      bool succed = false;
      while (succed != true) {
        succed = q.pop(item);
      }
      std::atomic_fetch_add(&popsum, item);
      /* std::cout << item << "\n"; */
    }
  });

  std::thread reader2([&]() {
    for (int i = 0; i < size / 2; i++) {
      int item = -1;
      bool succed = false;
      while (succed != true) {
        succed = q.pop(item);
      }
      std::atomic_fetch_add(&popsum, item);
      /* std::cout << item << "\n"; */
    }
  });

  writer1.join();
  writer2.join();
  reader1.join();
  reader2.join();
  assert(pushsum.load() == (size - 1) * size / 2);
  assert(popsum.load() == (size - 1) * size / 2);
}
int main() {
  test_in_searil();
  test_in_multithreads();
  return 0;
}
