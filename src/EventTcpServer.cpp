#include "EventTcpServer.h"
#include "EventSocket.h"
#include "EventSocketConnected.h"

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
    return 1;
  }
  TRACE(DBG) << "Starting TCP server";

  //To stop handling on SIGINT
  event * sigIntEvent = evsignal_new(_base.get(), SIGINT, &EventTcpServer::onSigInterruptStatic, this);
  event_add(sigIntEvent, NULL);

  int res = event_base_dispatch(_base.get());
  TRACE(DBG) << "Loop has ended with result: " << res;
  return res;
}

ISocksConnectionPtr EventTcpServer::addConnection(ISocksConnectionUser * user, const SocksAddress & addr)
{
  ISocksConnectionPtr ptr = std::make_shared<EventSocketConnected>(_base, addr);
  _connections[user] = ptr;
  ptr->setUser(user);
  return ptr;
}

void EventTcpServer::closeConnection(ISocksConnectionUser * user)
{
  auto it = _connections.find(user);
  if (it == _connections.end())
    return;
  it->second.reset();
}

void EventTcpServer::onAcceptConnectionStatic(evconnlistener * listener, evutil_socket_t fd,
                                              sockaddr * addr, int socklen, void * arg)
{
  auto * ptr = static_cast<EventTcpServer *>(arg);
  ptr->onAcceptConnection(listener, fd, addr, socklen);
}

void EventTcpServer::onSigInterruptStatic(evutil_socket_t fd, short what, void * arg)
{
  auto * ptr = static_cast<EventTcpServer *>(arg);
  ptr->onSigInterrupt(fd, what);
}

void EventTcpServer::onSigInterrupt(evutil_socket_t fd, short what)
{
  TRACE(INF) << "SIGINT received, stop event loop";
  event_base_loopbreak(_base.get());
}

void EventTcpServer::onAcceptConnection(evconnlistener * listener, evutil_socket_t fd, sockaddr * addr, int socklen)
{
  TRACE(DBG) << "On accept connection";

  EventSocket * newConn = new EventSocket(_base, fd);
  _user.onNewConnection(newConn);
}
