#ifndef SIGNALHANDLER_HPP
#define SIGNALHANDLER_HPP

#include <memory>
#include <tuple>
#include <atomic>
#include <thread>
#include "ConcurrentQueue.h"
#include "Private/SignalStorage.h"
#include <iostream>

class SignalHandler
{
public:
  enum class Error : int
  {
    NoError,
    EmptySignal
  };

public:
  SignalHandler();

  template<class ... Args>
  void push(std::shared_ptr<Callback> cb, std::tuple<Args...> && args)
  {
    ISignalStorage * sig = new SignalStorage<Args...>{cb, std::move(args)};
    _queue.push(sig);
    std::cout << "Push, size: " << _queue.size() << ", addr: " << &_queue << "\n";
  }

//  template<class ... Args>
//  void push(std::weak_ptr<Callback> cb, std::tuple<Args...> && args)
//  {
//    //ISignalStorage * sig = new SignalStorage<Args...>{cb, std::forward<Args>(args)...};
//    ISignalStorage * sig = new SignalStorage<Args...>{cb, std::move(args)};
//    _queue.push(sig);
//    std::cout << "Push, size: " << _queue.size() << ", addr: " << &_queue << "\n";
//  }

  int run(bool inNewThread);
  int runOnce();
  void stop();
  Error getError() const;

private:
  std::atomic<bool> _runFlag;
  Error _error;
  std::thread _thread;
  tools::ConcurrentQueue<ISignalStorage *> _queue;

  int runImpl(bool once);
  bool checkQueue();
  void executeSignal(ISignalStorage * sig);
  void setError(Error err);
};

#endif // SIGNALHANDLER_HPP
