#ifndef SIGNALSTORAGE_HPP
#define SIGNALSTORAGE_HPP

#include <memory>
#include <tuple>
#include <functional>
#include "SignalCallback.h"

struct ISignalStorage
{
  virtual ~ISignalStorage() {}
  virtual void call() = 0;
};

template<class ... Args>
class SignalStorage : public ISignalStorage
{
public:
  SignalStorage(std::weak_ptr<Callback> cb, std::tuple<Args...> && args) :
    _cb(cb),
    _args(std::move(args))
  {}

private:
  std::weak_ptr<Callback> _cb;
  std::tuple<Args...> _args;

  virtual void call() override
  {
    auto cb = _cb.lock();
    if (cb)
      std::apply(*cb, std::move(_args));
  }
};

#endif // SIGNALSTORAGE_HPP
