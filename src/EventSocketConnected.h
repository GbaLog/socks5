#ifndef EVENTSOCKETCONNECTED_H
#define EVENTSOCKETCONNECTED_H

#include "SocksInterfaces.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <Tracer.h>

class EventSocketConnected : public ISocksConnection, public Traceable
{
public:
  EventSocketConnected(event_base * base, SocksAddress addr);

  static void onReadStatic(bufferevent * bev, void * arg);
  static void onWriteStatic(bufferevent * bev, void * arg);
  static void onEventStatic(bufferevent * bev, short events, void * arg);

private:
  event_base * _base;
  evutil_socket_t _fd;
  typedef std::unique_ptr<bufferevent, void (*)(bufferevent *)> BufferEventPtr;
  BufferEventPtr _bev;
  ISocksConnectionUser * _user;
  SocksAddress _peerAddress;
  std::optional<SocksAddress> _localAddress;
  bool _waitForConnect;

  void onRead(bufferevent * bev);
  void onWrite(bufferevent * bev);
  void onEvent(bufferevent * bev, short events);
  std::optional<SocksAddress> getLocalAddressImpl() const;

  virtual void setUser(ISocksConnectionUser * user) override;
  virtual bool connect() override;
  virtual bool send(const VecByte & buf) override;
  virtual void closeConnection() override;
  virtual std::optional<SocksAddress> getLocalAddress() const override;
};

#endif // EVENTSOCKETCONNECTED_H
