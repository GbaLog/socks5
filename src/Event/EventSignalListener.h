#ifndef EventSignalListenerH
#define EventSignalListenerH
//-----------------------------------------------------------------------------
#include "EventBaseObject.h"
#include "EventSocketCommon.h"
#include <unordered_map>
//-----------------------------------------------------------------------------
class ISignalListenerUser
{
public:
  virtual ~ISignalListenerUser() = default;

  virtual void onSignalOccured(int signum) = 0;
};
//-----------------------------------------------------------------------------
class EventSignalListener
{
public:
  EventSignalListener(ISignalListenerUser & user, EventBaseObject & base);

  void add(int signum);
  void del(int signum);

private:
  ISignalListenerUser & _user;
  EventBasePtr _base;
  std::unordered_map<int, EventPtr> _events;

  static void onSignalStatic(evutil_socket_t fd, short what, void * arg);
};
//-----------------------------------------------------------------------------
#endif // EventSignalListenerH
//-----------------------------------------------------------------------------
