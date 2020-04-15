/*#include "Event2Server.h"

EventLoop::EventLoop()
{

}
*/
/*
// This file is a "Hello, world!" in C++ language by GCC for wandbox.
#include <iostream>
#include <cstdlib>
#include <functional>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <cstring>
#include <list>
#include <string_view>
#include <atomic>
#include <deque>
#include <condition_variable>
#include <thread>

#ifndef WRAPPER_H
#define WRAPPER_H

template<class T, class R, class ... Args>
class Wrapper
{
public:
  Wrapper(T * ptr, R (T::*func)(Args...)) :
    _ptr(ptr),
    _func(func)
  {}

  Wrapper(const Wrapper &) = default;
  Wrapper & operator =(const Wrapper &) = default;
  Wrapper(Wrapper &&) = default;
  Wrapper & operator =(Wrapper &&) = default;

  R operator ()(Args &&... args)
  {
    return _func(_ptr, std::forward<Args>(args)...);
  }

private:
  T * _ptr;
  std::function<R (T *, Args...)> _func;
};

template<class T, class R, class ... Args>
Wrapper<T, R, Args...> makeWrapper(T * ptr, R (T::*func)(Args...))
{
  return Wrapper<T, R, Args...>(ptr, func);
}

#endif //WRAPPER_H

template<class T, class R, class ... Args>
uint32_t makeHash(T * ptr, R (T::*func)(Args...))
{
  char storage[sizeof(func)];
  std::memcpy(storage, &func, sizeof(func));
  const char * data = reinterpret_cast<const char *>(&storage);

  uint32_t hash = std::hash<std::string_view>{}({data, sizeof(func)});
  hash += std::hash<T *>{}(ptr);
  return hash;
}


struct ICallbackFuncStorage
{
  virtual ~ICallbackFuncStorage() = default;
};

template<class R, class ... Args>
class CallbackFuncStorage : public ICallbackFuncStorage
{
public:
  template<class T>
  CallbackFuncStorage(T * ptr, R (T::*func)(Args...)) :
    _func(makeWrapper(ptr, func))
  {}

  CallbackFuncStorage(const CallbackFuncStorage &) = default;
  CallbackFuncStorage & operator =(const CallbackFuncStorage &) = default;

  R call(Args &&... args)
  {
    return _func(std::forward<Args>(args)...);
  }

private:
  std::function<R(Args...)> _func;
};

class Callback
{
public:
  Callback() :
    _funcStorage(nullptr),
    _cb(nullptr),
    _hash(0)
  {}

  template<class T, class R, class ... Args>
  Callback(T * ptr, R (T::*func) (Args...)) :
    _funcStorage(new CallbackFuncStorage<R, Args...>(ptr, func)),
    _cb(reinterpret_cast<void *>(&callbackTemplate<R, Args...>)),
    _hash(makeHash(ptr, func))
  {}

  Callback(Callback && rhs) :
    _funcStorage(std::move(rhs._funcStorage)),
    _cb(rhs._cb),
    _hash(rhs._hash)
  {
    rhs._cb = nullptr;
    rhs._hash = 0;
  }

  Callback & operator =(Callback && rhs)
  {
    _funcStorage = std::move(rhs._funcStorage);
    _cb = rhs._cb;
    _hash = rhs._hash;
    rhs._cb = nullptr;
    rhs._hash = 0;
    return *this;
  }

  template<class ... Args>
  void call(Args &&... args)
  {
    reinterpret_cast<void (*) (FuncStoragePtr &, Args &&...)>(_cb)(_funcStorage, std::forward<Args>(args)...);
  }

  template<class ... Args>
  void operator ()(Args &&... args)
  {
    call(std::forward<Args>(args)...);
  }

  uint32_t getHash() const { return _hash; }

private:
  using FuncStoragePtr = std::unique_ptr<ICallbackFuncStorage>;
  FuncStoragePtr _funcStorage;
  void * _cb;
  uint32_t _hash;

  template<class R, class ... Args>
  static void callbackTemplate(FuncStoragePtr & f, Args &&... args)
  {
    auto * func = reinterpret_cast<CallbackFuncStorage<R, Args...> *>(f.get());

    func->call(std::forward<Args>(args)...);
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

class SignalConnectionStorage
{
public:
  struct SignalPipe
  {
    void * _object;
    std::shared_ptr<Callback> _callback;
  };

  using SignalConnection = std::list<SignalPipe>;

public:
  static SignalConnectionStorage & instance()
  {
    static SignalConnectionStorage storage;
    return storage;
  }

  template<class T1, class T2, class R, class ... Args>
  bool connect(T1 * t1, R (T1::*f1)(Args...), T2 * t2, R (T2::*f2)(Args...))
  {
    auto it = _sigMap.find(makeHash(t1, f1));
    if (it == _sigMap.end())
    {
      SignalConnection conn;
      conn.push_back(SignalPipe{t2, std::make_shared<Callback>(t2, f2)});
      _sigMap.insert(std::make_pair(makeHash(t1, f1), std::move(conn)));
      return true;
    }

    auto h = makeHash(t2, f2);
    SignalConnection & conn = it->second;
    auto connIt = std::find_if(conn.begin(), conn.end(),
                               [h] (SignalPipe & p)
    { return p._callback->getHash() == h; });

    //If the signal already connected to this pointer, then reconnect with new callback
    if (connIt != conn.end())
    {
      auto cb = std::make_shared<Callback>(t2, f2);
      connIt->_callback = std::move(cb);
      return true;
    }

    conn.push_back(SignalPipe{t2, std::make_shared<Callback>(t2, f2)});
    return true;
  }

  template<class T1, class T2, class R, class ... Args>
  bool disconnect(T1 * t1, R (T1::*f1)(Args...), T2 * t2, R (T2::*f2)(Args...))
  {
    auto it = _sigMap.find(makeHash(t1, f1));
    if (it == _sigMap.end())
      return false;

    auto h = makeHash(t2, f2);
    SignalConnection & conn = it->second;
    auto connIt = std::find_if(conn.begin(), conn.end(),
                               [h] (SignalPipe & p)
    { return p._callback->getHash() == h; });

    if (connIt == conn.end())
      return false;
    conn.erase(connIt);

    if (conn.empty())
      _sigMap.erase(it);
    return true;
  }

  template<class T>
  bool disconnectByHash(uint32_t hash, const T * ptr)
  {
    auto it = _sigMap.find(hash);
    if (it == _sigMap.end())
      return false;

    SignalConnection & conn = it->second;
    conn.erase(std::remove_if(conn.begin(), conn.end(),
                              [ptr] (const SignalPipe & p) { return p._object == ptr; })
               , conn.end());
    return true;
  }

  template<class T, class F>
  bool hasConnection(T * ptr, F func) const
  {
    return _sigMap.find(makeHash(ptr, func)) != _sigMap.end();
  }

  template<class T, class R, class ... Args>
  SignalConnection & getConnection(T * ptr, R (T::*func)(Args...))
  {
    auto it = _sigMap.find(makeHash(ptr, func));
    if (it == _sigMap.end())
      throw std::runtime_error{"Cannot find connection for pointer"};
    return it->second;
  }

private:
  using SignalMap = std::unordered_map<uint32_t, SignalConnection>;
  SignalMap _sigMap;

  SignalConnectionStorage() = default;
  ~SignalConnectionStorage() = default;
};

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
    if (_cb.expired() == false)
      std::apply(*_cb.lock(), std::move(_args));
  }
};

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
    std::unique_lock lock(_mutex);
    return _queue.empty();
  }

private:
  std::condition_variable _cv;
  mutable std::mutex _mutex;
  std::deque<T> _queue;
};
}
namespace wdha {

class SignalHandler
{
public:
  enum class Error : int
  {
    NoError,
    EmptySignal
  };

public:
  SignalHandler() :
    _runFlag(false),
    _error(Error::NoError)
  {}

  template<class ... Args>
  void push(std::weak_ptr<Callback> cb, std::tuple<Args...> && args)
  {
    std::cout << "Push signal\n";
    //ISignalStorage * sig = new SignalStorage<Args...>{cb, std::forward<Args>(args)...};
    ISignalStorage * sig = new SignalStorage<Args...>{cb, std::move(args)};
    _queue.push(sig);
  }

  int run(bool inNewThread)
  {
    if (_runFlag)
      return 1;

    _runFlag = true;
    if (inNewThread)
    {
      _thread = std::thread(&SignalHandler::runImpl, this, false);
      return 0;
    }
    else
      return runImpl(false);
  }

  int runOnce()
  {
    return runImpl(true);
  }

  void stop()
  {
    _runFlag = false;
  }

  Error getError() const
  {
    return _error;
  }

private:
  std::atomic<bool> _runFlag;
  Error _error;
  std::thread _thread;
  tools::ConcurrentQueue<ISignalStorage *> _queue;

  int runImpl(bool once)
  {
    std::cout << "Run impl\n";
    if (once)
    {
      ISignalStorage * sig = nullptr;
      while (_queue.empty() == false)
      {
        sig = _queue.pop();
        executeSignal(sig);
      }
      return static_cast<int>(getError());
    }

    while (_runFlag)
    {
      std::cout << "Tick\n";
      if (!checkQueue())
        break;
    }
    return static_cast<int>(getError());
  }

  bool checkQueue()
  {
    ISignalStorage * sig = _queue.pop();
    if (!sig)
    {
      setError(Error::EmptySignal);
      return false;
    }
    executeSignal(sig);
    return true;
  }

  void executeSignal(ISignalStorage * sig)
  {
    if (!sig)
      return;
    sig->call();
    delete sig;
    std::cout << "Take signal\n";
  }

  void setError(Error err)
  { _error = err; }
};

}
class Object
{
public:

  Object(SignalHandler & handler, SignalConnectionStorage & connStorage = SignalConnectionStorage::instance()) :
    _handler(handler),
    _connStorage(connStorage)
  {}

  ~Object() { unsubscribeFromAll(); }

  template<class T1, class T2, class R, class ... Args>
  void subscribe(T1 * t1, R (T1::*f1)(Args...), T2 * t2, R (T2::*f2)(Args...))
  {
    if (_connStorage.connect(t1, f1, t2, f2) == true)
      _hashes.insert(makeHash(t1, f1));
  }

  template<class T1, class T2, class R, class ... Args>
  void unsubscribe(T1 * t1, R (T1::*f1)(Args...), T2 * t2, R (T2::*f2)(Args...))
  {
    if (_connStorage.disconnect(t1, f1, t2, f2) == true)
      _hashes.erase(makeHash(t1, f1));
  }

  template<class T, class R, class ... Args, class ... Args2>
  void emitAll(T * t, R (T::*f)(Args...), Args2 &&... args)
  {
    if (_connStorage.hasConnection(t, f) == false)
      return;
    auto & conn = _connStorage.getConnection(t, f);
    if (conn.size() == 1)
    {
      _handler.push<Args...>(conn.front()._callback, std::make_tuple(std::forward<Args2>(args)...));
      //conn.front()._callback.call(std::forward<Args>(args)...);
      return;
    }

    for (auto & it : conn)
    {
      //_handler.push<Args...>(&it._callback, args...);
      //it._callback.call(args...);
    }
  }

private:
  SignalHandler & _handler;
  SignalConnectionStorage & _connStorage;

  std::unordered_set<uint32_t> _hashes;

  //TODO: don't store hashes here
  //Store them into connection storage like obj_ptr -> hash
  void unsubscribeFromAll()
  {
    for (const auto it : _hashes)
      _connStorage.disconnectByHash(it, this);
  }
};


#define DECL_SIGNAL(name, ...) void name(__VA_ARGS__) {}
#define EMIT_ALL(method, ...) Object::emitAll(this, &std::remove_reference_t<decltype(*this)>::method, ##__VA_ARGS__)

class A : public Object
{
public:
  explicit A(SignalHandler & handler) : Object(handler) {}

  DECL_SIGNAL(signal, int)
  DECL_SIGNAL(signal2)

  void foo(int n)
  {
    EMIT_ALL(signal, n);
    EMIT_ALL(signal2);
  }
};

class B : public Object
{
public:
  B(SignalHandler & handler, A & a) :
    Object(handler),
    _a(a)
  {
    subscribe(&a, &A::signal, this, &B::recvSignal);
  }

  ~B() {}

private:
  void recvSignal(int n)
  {
    std::cout << "N = " << n << "\n";
    if (n < 40)
    {
      subscribe(&_a, &A::signal, this, &B::recvSignal2);
    }
  }

  void recvSignal2(int n)
  {
    std::cout << __PRETTY_FUNCTION__ << ", n: " << n << std::endl;
    unsubscribe(&_a, &A::signal, this, &B::recvSignal);

    static int counter = 0;
    if (++counter > 3)
    {
      subscribe(&_a, &A::signal, this, &B::recvSignal);
    }
  }

private:
  A & _a;
};

class C : public Object
{
public:
  explicit C(SignalHandler & handler) :
    Object(handler),
    _handler(handler)
  {}

  void recvSignal()
  {
    std::cout << "Recv signal in C, stop handler\n";
    _handler.stop();
  }

private:
  SignalHandler & _handler;
};

int main()
{
  std::cout << "Hello, Wandbox!" << std::endl;

  SignalHandler handler;
  A a(handler);
  {
    B b(handler, a);
    a.foo(42);
    a.foo(32);
    a.foo(99);
    a.foo(-99);
    a.foo(1);
    a.foo(2);
    a.foo(3);
    a.foo(4);
    handler.runOnce();
  }
  C c(handler);
  static_cast<Object *>(&c)->subscribe(&a, &A::signal2, &c, &C::recvSignal);
  a.foo(69);
  return handler.run(false);
}
*/
