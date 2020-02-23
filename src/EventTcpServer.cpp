#include "EventTcpServer.h"
#include "EventSocket.h"

EventTcpServer::EventTcpServer(ITcpServerUser & user, sockaddr_in saddr) :
  Traceable("EvTcpSrv"),
  _user(user),
  _base(event_base_new(), event_base_free),
  _listener(evconnlistener_new_bind(_base.get(), &EventTcpServer::onAcceptConnectionStatic, this,
                                    LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                    (sockaddr *)&saddr, sizeof(saddr)),
            evconnlistener_free)
{}

int EventTcpServer::run()
{
  if (_listener == nullptr)
  {
    TRACE(ERR) << "Listener is not started";
    return false;
  }

  return event_base_dispatch(_base.get());
}

void EventTcpServer::onAcceptConnectionStatic(evconnlistener * listener, evutil_socket_t fd,
                                              sockaddr * addr, int socklen, void * arg)
{
  auto * ptr = static_cast<EventTcpServer *>(arg);
  ptr->onAcceptConnection(listener, fd, addr, socklen);
}

void EventTcpServer::onAcceptConnection(evconnlistener * listener, intptr_t fd, sockaddr * addr, int socklen)
{
  TRACE(DBG) << "On accept connection";

  bufferevent * bufferEvent = bufferevent_socket_new(_base.get(), fd, BEV_OPT_CLOSE_ON_FREE);

  EventSocket * newConn = new EventSocket(_base.get(), bufferEvent, fd);
  bufferevent_setcb(bufferEvent, &EventSocket::onReadStatic, &EventSocket::onWriteStatic,
                    &EventSocket::onEventStatic, newConn);
  bufferevent_enable(bufferEvent, EV_READ | EV_WRITE);
  _user.onNewConnection(newConn);
}
