#ifndef __ARRAY_LOCK_FREE_QUEUE_H__
#define __ARRAY_LOCK_FREE_QUEUE_H__

#include <atomic>
#include <cstdint>
#include <sched.h>

// MAX_SIZE must be (power of 2) -1
template <typename T, uint32_t MAX_SIZE = 31> class ArrayLockFreeQueue {
public:
  ArrayLockFreeQueue() : writeIndex(0), readIndex(0), maximumReadIndex(0) {}
  ~ArrayLockFreeQueue() {}

  bool push(const T &item) {
    uint32_t currentReadIndex = 0;
    uint32_t currentWriteIndex = 0;
    uint32_t newWriteInex = 0;

    // reserve space to commit
    do {
      currentWriteIndex = writeIndex.load();
      currentReadIndex = readIndex.load();
      newWriteInex = (currentWriteIndex + 1) & sizeMask;
      if (newWriteInex == currentReadIndex) {
        return false;
      }

    } while (!std::atomic_compare_exchange_weak(&writeIndex, &currentWriteIndex,
                                                newWriteInex));

    // commit data
    data[currentWriteIndex] = item;

    while (!std::atomic_compare_exchange_weak(
        &maximumReadIndex, &currentWriteIndex, newWriteInex)) {
      // the commit step should be order between threads
      // so when producers more than the cores, yield is needed to avoid spinlock.
      sched_yield();
    }

    return true;
  }

  bool pop(T &item) {

    uint32_t currentMaximumReadIndex = 0;
    uint32_t currentReadIndex = 0;
    uint32_t newReadIndex = 0;

    do {
      currentReadIndex = readIndex.load();
      currentMaximumReadIndex = maximumReadIndex.load();

      if (currentReadIndex == currentMaximumReadIndex) {
        return false;
      }

      // this read will not drop the item in data
      item = data[currentReadIndex];

      newReadIndex = (currentReadIndex + 1) & sizeMask;
      // if cas fail, it means that this item is consumed by other consumer.
      if (std::atomic_compare_exchange_weak(&readIndex, &currentReadIndex,
                                            newReadIndex)) {
        return true;
      }
    } while (1);

    // Add this return statement to avoid compiler warnings
    return false;
  }

  T data[MAX_SIZE + 1];

  const uint32_t sizeMask = MAX_SIZE;

  std::atomic<uint32_t> writeIndex;

  std::atomic<uint32_t> readIndex;

  std::atomic<uint32_t> maximumReadIndex;
};

#endif // __ARRAY_LOCK_FREE_QUEUE_H__
