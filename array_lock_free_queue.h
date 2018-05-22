#ifndef __ARRAY_LOCK_FREE_QUEUE_H__
#define __ARRAY_LOCK_FREE_QUEUE_H__

#include <atomic>
#include <thread>

// MAX_SIZE must be (power of 2) -1, then we can count new index by bit
// operation.
template <typename T, size_t MAX_SIZE = 31> class ArrayLockFreeQueue {
public:
  ArrayLockFreeQueue() : writeIndex(0), readIndex(0), maximumReadIndex(0) {}
  ~ArrayLockFreeQueue() {}

  bool push(const T &item) {
    size_t currentReadIndex = 0;
    size_t currentWriteIndex = 0;
    size_t nextWriteInex = 0;

    // reserve space to commit
    do {
      currentWriteIndex = writeIndex.load();
      currentReadIndex = readIndex.load();
      nextWriteInex = (currentWriteIndex + 1) & sizeMask;
      if (nextWriteInex == currentReadIndex) {
        // queue full
        return false;
      }

      // cas fail means other producer take this place, this thread need try
      // again.
    } while (!std::atomic_compare_exchange_weak(&writeIndex, &currentWriteIndex,
                                                nextWriteInex));

    // commit data to the reserve place
    data[currentWriteIndex] = item;

    while (!std::atomic_compare_exchange_weak(
        &maximumReadIndex, &currentWriteIndex, nextWriteInex)) {
      // the commit step should be order between producers, producer 1 ->
      // producer 2 -> producer 3. so when producers more than the cores, yield
      // is needed to avoid spinlock.
       std::this_thread::yield();
    }

    return true;
  }

  bool pop(T &item) {

    size_t currentMaximumReadIndex = 0;
    size_t currentReadIndex = 0;
    size_t nextReadIndex = 0;

    do {
      currentReadIndex = readIndex.load();
      currentMaximumReadIndex = maximumReadIndex.load();

      if (currentReadIndex == currentMaximumReadIndex) {
        // queue empty
        return false;
      }

      // this read will not drop the item in data, so other consumer can still read this item.
      item = data[currentReadIndex];

      nextReadIndex = (currentReadIndex + 1) & sizeMask;
      // cas fail means this item is consumed by other consumer.
    } while (!std::atomic_compare_exchange_weak(&readIndex, &currentReadIndex,
                                                nextReadIndex));

    return true;
  }

  T data[MAX_SIZE + 1];

  const size_t sizeMask = MAX_SIZE;

  std::atomic<size_t> writeIndex;

  std::atomic<size_t> readIndex;

  std::atomic<size_t> maximumReadIndex;
};

#endif // __ARRAY_LOCK_FREE_QUEUE_H__
