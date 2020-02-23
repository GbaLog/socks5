#ifndef EVENTTCPSERVER_H
#define EVENTTCPSERVER_H

#include <memory>
#include "Tracer.h"
#include "SocksInterfaces.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

class EventTcpServer : Traceable
{
public:
  EventTcpServer(ITcpServerUser & user, sockaddr_in saddr);

  int run();

private:
  ITcpServerUser & _user;

  typedef std::unique_ptr<event_base, void (*)(event_base *)> EventBasePtr;
  EventBasePtr _base;
  typedef std::unique_ptr<evconnlistener, void (*)(evconnlistener *)> EventListenerPtr;
  EventListenerPtr _listener;

  static void onAcceptConnectionStatic(evconnlistener * listener, evutil_socket_t fd,
                                       sockaddr * addr, int socklen, void * arg);
  void onAcceptConnection(evconnlistener * listener, evutil_socket_t fd, sockaddr * addr, int socklen);
};

#endif // EVENTTCPSERVER_H
