#ifndef EVENTSOCKET_H
#define EVENTSOCKET_H

#include "SocksInterfaces.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include "Tracer.h"

class EventSocket : public ISocksConnection, private Traceable
{
public:
  EventSocket(event_base * base, evutil_socket_t fd);

private:
  event_base * _base;
  evutil_socket_t _fd;
  typedef std::unique_ptr<bufferevent, void (*)(bufferevent *)> BufferEventPtr;
  BufferEventPtr _bev;
  ISocksConnectionUser * _user;

  void onRead(bufferevent * bev);
  void onWrite(bufferevent * bev);
  void onEvent(bufferevent * bev, short events);

  virtual void setUser(ISocksConnectionUser * user) override;
  virtual bool connect() override;
  virtual bool send(const VecByte & buf) override;
  virtual void closeConnection() override;
  virtual std::optional<SocksAddress> getLocalAddress() const override;

  static void onReadStatic(bufferevent * bev, void * arg);
  static void onWriteStatic(bufferevent * bev, void * arg);
  static void onEventStatic(bufferevent * bev, short events, void * arg);
};

#endif // EVENTSOCKET_H
