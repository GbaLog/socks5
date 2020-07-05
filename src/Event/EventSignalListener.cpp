#include "EventSignalListener.h"
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
void freeEvent(event * ev)
{
  if (!ev) return;
  event_del(ev);
  event_free(ev);
}
//-----------------------------------------------------------------------------
EventPtr makeSinalEvent(event * ptr)
{
  return EventPtr{ptr, freeEvent};
}
//-----------------------------------------------------------------------------
} //namespace
//-----------------------------------------------------------------------------
EventSignalListener::EventSignalListener(ISignalListenerUser & user, EventBaseObject & base) :
  _user(user),
  _base(base.get())
{}
//-----------------------------------------------------------------------------
void EventSignalListener::add(int signum)
{
  auto it = _events.find(signum);
  if (it != _events.end())
  {
    _events.erase(it);
  }

  event * ev = evsignal_new(_base.get(), signum, onSignalStatic, this);
  if (ev == nullptr)
    return;

  event_add(ev, NULL);
  _events.emplace(signum, makeSinalEvent(ev));
}
//-----------------------------------------------------------------------------
void EventSignalListener::del(int signum)
{
  _events.erase(signum);
}
//-----------------------------------------------------------------------------
void EventSignalListener::onSignalStatic(evutil_socket_t fd, short what, void * arg)
{
  auto * ptr = (EventSignalListener *)arg;
  ptr->_user.onSignalOccured((int)fd);
}
//-----------------------------------------------------------------------------
