#include "EventTcpServer.h"
#include "EventSocket.h"
#include "EventSocketConnected.h"
#include <signal.h>
#ifdef _WIN32
#include <ws2ipdef.h>
#endif //WIN32
//-----------------------------------------------------------------------------
EventTcpServer::EventTcpServer(ITcpServerUser & user, sockaddr_in saddr) :
  LoggerAdapter("EvTcpSrv"),
  _user(user),
  _base(event_base_new(), event_base_free),
  _listener(evconnlistener_new_bind(_base.get(), &EventTcpServer::onAcceptConnectionStatic, this,
                                    LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                    (sockaddr *)&saddr, sizeof(saddr)),
            evconnlistener_free)
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

  //To stop handling on SIGINT
  event * sigIntEvent = evsignal_new(_base.get(), SIGINT, &EventTcpServer::onSigInterruptStatic, this);
  event_add(sigIntEvent, NULL);

  int res = event_base_dispatch(_base.get());
  log(DBG, "Loop has ended with result: {}", res);
  return res;
}
//-----------------------------------------------------------------------------
ISocksConnectionPtr EventTcpServer::addConnection(ISocksConnectionUser * user, const SocksAddress & addr)
{
  ISocksConnectionPtr ptr = std::make_shared<EventSocketConnected>(_base, addr);
  _connections[user] = ptr;
  ptr->setUser(user);
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
void EventTcpServer::onSigInterruptStatic(evutil_socket_t fd, short what, void * arg)
{
  auto * ptr = static_cast<EventTcpServer *>(arg);
  ptr->onSigInterrupt(fd, what);
}
//-----------------------------------------------------------------------------
void EventTcpServer::onSigInterrupt(evutil_socket_t fd, short what)
{
  log(INF, "SIGINT received, stop event loop");
  event_base_loopbreak(_base.get());
}
//-----------------------------------------------------------------------------
void EventTcpServer::onAcceptConnection(evconnlistener * listener, evutil_socket_t fd, sockaddr * addr, int socklen)
{
  log(DBG, "On accept connection from addr: {}, port {}", getAddrStr(addr), getAddrPort(addr));

  EventSocket * newConn = new EventSocket(_base, fd);
  _user.onNewConnection(newConn);
}
//-----------------------------------------------------------------------------
std::string_view EventTcpServer::getAddrStr(sockaddr * addr)
{
  static thread_local char buf[46];
  if (addr->sa_family == AF_INET6)
  {
    sockaddr_in6 * in6 = (sockaddr_in6 *)addr;
    return evutil_inet_ntop(AF_INET6, &in6->sin6_addr, buf, sizeof(buf));
  }
  sockaddr_in * in = (sockaddr_in *)addr;
  return evutil_inet_ntop(AF_INET, &in->sin_addr, buf, sizeof(buf));
}
//-----------------------------------------------------------------------------
uint16_t EventTcpServer::getAddrPort(sockaddr * addr)
{
  if (addr->sa_family == AF_INET6)
  {
    sockaddr_in6 * in6 = (sockaddr_in6 *)addr;
    return ntohs(in6->sin6_port);
  }

  sockaddr_in * in = (sockaddr_in *)addr;
  return ntohs(in->sin_port);
}
//-----------------------------------------------------------------------------
