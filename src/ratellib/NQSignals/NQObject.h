#ifndef NQObjectH
#define NQObjectH

#include <type_traits>
#include <unordered_set>
#include "Private/SignalHandler.h"
#include "Private/SignalConnectionStorage.h"
#include "Private/SignalHash.h"
#include "NQCoreApplication.h"

#define DECL_SIGNAL(name, ...) public: \
  void name(__VA_ARGS__) {}
#define EMIT_ALL(method, ...) NQObject::emitAll(this, &std::remove_reference_t<decltype(*this)>::method, ##__VA_ARGS__)

class NQObject
{
public:
  NQObject(SignalConnectionStorage & connStorage = SignalConnectionStorage::instance()) :
    _handler(NQCoreApplication::getSignalHandler()),
    _connStorage(connStorage)
  {}

  ~NQObject() { unsubscribeFromAll(); }

  template<class T1, class T2, class R, class ... Args>
  bool connect(T1 * t1, R (T1::*f1)(Args...), T2 * t2, R (T2::*f2)(Args...))
  {
    if (_connStorage.connect(t1, f1, t2, f2) == true)
    {
      _hashes.insert(makeHash(t1, f1));
      return true;
    }

    return false;
  }

  template<class T1, class T2, class R, class ... Args>
  bool disconnect(T1 * t1, R (T1::*f1)(Args...), T2 * t2, R (T2::*f2)(Args...))
  {
    if (_connStorage.disconnect(t1, f1, t2, f2) == true)
    {
      _hashes.erase(makeHash(t1, f1));
      return true;
    }

    return false;
  }

  template<class T, class R, class ... Args, class ... Args2>
  void emitAll(T * t, R (T::*f)(Args...), Args2 &&... args)
  {
    if (_connStorage.hasConnection(t, f) == false)
      return;

    auto cb = std::make_shared<Callback>(this, NQObject::onSignalEmmited<std::weak_ptr<Callback>, uint32_t, Args...>);
    auto tup = std::make_tuple(std::weak_ptr<Callback>(cb), makeHash(t, f), args...);
    _waitingSignals.insert(cb);

    _handler.push<std::weak_ptr<Callback>, uint32_t, Args...>(cb, std::move(tup));
  }

private:
  SignalHandler & _handler;
  SignalConnectionStorage & _connStorage;

  std::unordered_set<uint32_t> _hashes;
  std::set<std::shared_ptr<Callback>> _waitingSignals;

  template<class ... Args>
  void onSignalEmmited(std::weak_ptr<Callback> ptr, uint32_t hash, Args &&... args)
  {
    auto cb = ptr.lock();
    if (cb == nullptr)
      return;
    _waitingSignals.erase(cb);

    if (_connStorage.hasConnection(hash) == false)
      return;

    auto & conn = _connStorage.getConnection(hash);
    if (conn.size() == 1)
    {
      (*conn.front()._callback)(std::move(args)...);
      return;
    }

    for (auto & it : conn)
    {
      (*it._callback)(args...);
    }
  }

  //TODO: don't store hashes here
  //Store them into connection storage like obj_ptr -> hash
  void unsubscribeFromAll()
  {
    for (const auto it : _hashes)
      _connStorage.disconnectByHash(it, this);
  }
};

#endif // NQObjectH
