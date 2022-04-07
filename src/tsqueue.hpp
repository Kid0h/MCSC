#ifndef TSQUEUE_HPP
#define TSQUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class TSQueue
{
public:
  TSQueue(void)
    : q()
    , m()
    , c()
  {}

  ~TSQueue(void)
  {}

  // Add an element to the queue.
  void enqueue(const T& t) {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }
  void enqueue(T&& t) {
    std::lock_guard<std::mutex> lock(m);
    q.push(std::move(t));
    c.notify_one();
  }


  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T dequeue(void) {
    std::unique_lock<std::mutex> lock(m);
    while(q.empty())
    {
      // release lock as long as the wait and reaquire it afterwards.
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

  bool empty() {
    std::unique_lock<std::mutex> lock(m);
    return q.empty();
  }

  auto size() {
    std::unique_lock<std::mutex> lock(m);
    return q.size();
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};

#endif