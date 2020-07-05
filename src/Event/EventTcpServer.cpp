#include "EventTcpServer.h"
#include "EventSocket.h"
#include "EventSocketConnected.h"
#include <signal.h>
//-----------------------------------------------------------------------------
EventTcpServer::EventTcpServer(ITcpServerUser & user, EventBaseObject & base,
                               const sockaddr * saddr, int salen) :
  LoggerAdapter("EvTcpSrv"),
  _user(user),
  _base(base.get()),
  _listener(makeListener(_base, onAcceptConnectionStatic, this, saddr, salen))
{}
//-----------------------------------------------------------------------------
int EventTcpServer::run()
{
  if (_listener == nullptr)
  {
    log(ERR, "Listener is not started");
    return 1;
  }
  log(DBG, "Starting TCP server");

  int res = event_base_dispatch(_base.get());
  log(DBG, "Loop has ended with result: {}", res);
  return res;
}
//-----------------------------------------------------------------------------
void EventTcpServer::stop()
{
  log(INF, "SIGINT received, stop event loop");
  event_base_loopbreak(_base.get());
}
//-----------------------------------------------------------------------------
SocksConnectionPtr EventTcpServer::addConnection(ISocksConnectionUser * user, const SocksAddress & addr)
{
  SocksConnectionPtr ptr = EventSocketConnected::createConnect(_base, addr, user);
  if (ptr == nullptr)
    return nullptr;

  _connections[user] = ptr;
  return ptr;
}
//-----------------------------------------------------------------------------
void EventTcpServer::closeConnection(ISocksConnectionUser * user)
{
  auto it = _connections.find(user);
  if (it == _connections.end())
    return;
  it->second.reset();
}
//-----------------------------------------------------------------------------
void EventTcpServer::onAcceptConnectionStatic(evconnlistener * listener, evutil_socket_t fd,
                                              sockaddr * addr, int socklen, void * arg)
{
  auto * ptr = static_cast<EventTcpServer *>(arg);
  ptr->onAcceptConnection(listener, fd, addr, socklen);
}
//-----------------------------------------------------------------------------
void EventTcpServer::onAcceptConnection(evconnlistener * listener, evutil_socket_t fd, sockaddr * addr, int socklen)
{
  log(DBG, "On accept connection from addr: {}, port {}", getAddrStr(addr), getAddrPort(addr));

  SocksConnectionPtr newConn = SocksConnectionPtr{new EventSocketConnected(_base, fd)};
  _user.onNewConnection(newConn);
}
//-----------------------------------------------------------------------------
