#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <deque>
#include <condition_variable>
#include <mutex>

namespace tools
{

template<class T>
class ConcurrentQueue
{
public:
  void push(const T & val)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push_back(val);
    lock.unlock();
    _cv.notify_one();
  }

  bool tryPop(T & val)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty()) return false;
    val = _queue.front();
    _queue.pop_front();
    return true;
  }

  T pop()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_queue.empty())
    {
      _cv.wait(lock);
    }
    T val = _queue.front();
    _queue.pop_front();
    return val;
  }

  bool empty() const
  {
    std::unique_lock<std::mutex> lock(_mutex);
    return _queue.empty();
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(_mutex);
    return _queue.size();
  }

private:
  std::condition_variable _cv;
  mutable std::mutex _mutex;
  std::deque<T> _queue;
};

}

#endif // CONCURRENTQUEUE_H
