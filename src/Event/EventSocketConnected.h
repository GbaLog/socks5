#ifndef EventSocketConnectedH
#define EventSocketConnectedH
//-----------------------------------------------------------------------------
#include "SocksInterfaces.h"
#include "EventSocketCommon.h"
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include "LoggerAdapter.h"
//-----------------------------------------------------------------------------
class EventSocketConnected : public ISocksConnection, private LoggerAdapter
{
public:
  EventSocketConnected(EventBasePtr base, evutil_socket_t fd);

  static SocksConnectionPtr createConnect(EventBasePtr base,
                                            const SocksAddress & addr, ISocksConnectionUser * user);

private:
  typedef std::unique_ptr<bufferevent, void (*)(bufferevent *)> BufferEventPtr;
  BufferEventPtr _bev;
  ISocksConnectionUser * _user;
  SocksAddress _remoteAddress;
  SocksAddress _localAddress;
  bool _connected;

  EventSocketConnected(EventBasePtr base, const SocksAddress & addr);

  void onRead(bufferevent * bev);
  void onWrite(bufferevent * bev);
  void onEvent(bufferevent * bev, short events);
  void doShutdown();
  void updateLocalAddress();
  void updateRemoteAddress();
  void enable();
  bool connect();

  virtual void setUser(ISocksConnectionUser * user) override;
  virtual bool send(const VecByte & buf) override;
  virtual void closeConnection() override;
  virtual bool isConnected() const override;
  virtual std::optional<SocksAddress> getLocalAddress() const override;

  static void onReadStatic(bufferevent * bev, void * arg);
  static void onWriteStatic(bufferevent * bev, void * arg);
  static void onEventStatic(bufferevent * bev, short events, void * arg);
};
//-----------------------------------------------------------------------------
#endif // EventSocketConnectedH
//-----------------------------------------------------------------------------
