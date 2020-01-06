#ifndef SIGNALCALLBACK_HPP
#define SIGNALCALLBACK_HPP

#include <functional>
#include <any>
#include "Wrapper.h"
#include "SignalHash.h"

template<class R, class ... Args>
class CallbackFuncStorage
{
public:
  template<class T>
  CallbackFuncStorage(T * ptr, R (T::*func)(Args...)) :
    _func(makeWrapper(ptr, func))
  {}

  CallbackFuncStorage(R (*func)(Args...)) :
    _func(func)
  {}

  R operator ()(Args &&... args)
  {
    return _func(std::forward<Args>(args)...);
  }

private:
  std::function<R (Args...)> _func;
};

class Callback
{
public:
  Callback() :
    _cb(nullptr),
    _hash(0)
  {}

  template<class T, class R, class ... Args>
  Callback(T * ptr, R (T::*func) (Args...)) :
    _funcStorage(CallbackFuncStorage<R, Args...>(ptr, func)),
    _cb(reinterpret_cast<void *>(&callbackTemplate<R, Args...>)),
    _hash(makeHash(ptr, func))
  {}

  template<class R, class ... Args>
  Callback(R (*func)(Args...)) :
    _funcStorage(CallbackFuncStorage<R, Args...>(func)),
    _cb(reinterpret_cast<void *>(&callbackTemplate<R, Args...>)),
    _hash(makeHash(func))
  {}

  template<class ... Args>
  void operator ()(Args &&... args)
  {
    reinterpret_cast<void (*) (std::any &, Args &&...)>(_cb)(_funcStorage, std::forward<Args>(args)...);
  }

  uint32_t getHash() const { return _hash; }

private:
  std::any _funcStorage;
  void * _cb;
  uint32_t _hash;

  template<class R, class ... Args>
  static void callbackTemplate(std::any & f, Args &&... args)
  {
    auto func = std::any_cast<CallbackFuncStorage<R, Args...>>(f);
    func(std::forward<Args>(args)...);
  }
};

inline bool operator ==(const Callback & lhs, const Callback & rhs) noexcept
{ return lhs.getHash() == rhs.getHash(); }

namespace std
{
  template<> struct hash<Callback>
  {
    std::size_t operator ()(const Callback & s) const noexcept
    { return s.getHash(); }
  };
}

#endif // SIGNALCALLBACK_HPP
