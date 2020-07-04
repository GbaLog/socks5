#ifndef EVENTTCPSERVER_H
#define EVENTTCPSERVER_H

#include <memory>
#include "LoggerAdapter.h"
#include "SocksInterfaces.h"
#include "EventSocketCommon.h"
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <unordered_map>

class EventTcpServer : private LoggerAdapter
{
public:
  EventTcpServer(ITcpServerUser & user, sockaddr_in saddr);

  int run();
  ISocksConnectionPtr addConnection(ISocksConnectionUser * user, const SocksAddress & addr);
  void closeConnection(ISocksConnectionUser * user);

private:
  ITcpServerUser & _user;

  EventBasePtr _base;
  typedef std::unique_ptr<evconnlistener, void (*)(evconnlistener *)> EventListenerPtr;
  EventListenerPtr _listener;
  typedef std::unordered_map<ISocksConnectionUser *, ISocksConnectionPtr> MapConnections;
  MapConnections _connections;

  static void onAcceptConnectionStatic(evconnlistener * listener, evutil_socket_t fd,
                                       sockaddr * addr, int socklen, void * arg);
  static void onSigInterruptStatic(evutil_socket_t fd, short what, void * arg);
  void onSigInterrupt(evutil_socket_t fd, short what);
  void onAcceptConnection(evconnlistener * listener, evutil_socket_t fd, sockaddr * addr, int socklen);

  static std::string_view getAddrStr(sockaddr * addr);
  static uint16_t getAddrPort(sockaddr * addr);
};

#endif // EVENTTCPSERVER_H
