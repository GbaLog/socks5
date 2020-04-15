#ifndef SIGNALCONNECTIONSTORAGE_HPP
#define SIGNALCONNECTIONSTORAGE_HPP

#include <memory>
#include <list>
#include <set>
#include <unordered_map>
#include <algorithm>
#include "SignalCallback.h"

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
  static SignalConnectionStorage & instance();

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

    //FIXME: Hmm... This seems meaningless
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

  bool hasConnection(uint32_t hash) const;

  template<class T, class F>
  bool hasConnection(T * ptr, F func) const
  {
    return hasConnection(makeHash(ptr, func));
  }

  SignalConnection & getConnection(uint32_t hash);

  template<class T, class R, class ... Args>
  SignalConnection & getConnection(T * ptr, R (T::*func)(Args...))
  {
    return getConnection(makeHash(ptr, func));
  }

private:
  using SignalMap = std::unordered_map<uint32_t, SignalConnection>;
  SignalMap _sigMap;

  SignalConnectionStorage() = default;
  ~SignalConnectionStorage() = default;
};

#endif // SIGNALCONNECTIONSTORAGE_HPP
