#include "EventSocketConnected.h"
#include <cstring>
//-----------------------------------------------------------------------------
#ifdef _WIN32
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
//-----------------------------------------------------------------------------
EventSocketConnected::EventSocketConnected(EventBasePtr base, SocksAddress addr) :
  LoggerAdapter("EvSockConn"),
  _base(base),
  _fd(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
  _bev(bufferevent_socket_new(_base.get(), _fd, BEV_OPT_CLOSE_ON_FREE),
       bufferevent_free),
  _user(nullptr),
  _peerAddress(addr),
  _waitForConnect(false),
  _connected(false)
{
  evutil_make_socket_nonblocking(_fd);
  bufferevent_setcb(_bev.get(), NULL, NULL,
                    &EventSocketConnected::onEventStatic, this);
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onReadStatic(bufferevent * bev, void * arg)
{
  auto * ptr = (EventSocketConnected *)arg;
  ptr->onRead(bev);
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onWriteStatic(bufferevent * bev, void * arg)
{
  auto * ptr = (EventSocketConnected *)arg;
  ptr->onWrite(bev);
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onEventStatic(bufferevent * bev, short events, void * arg)
{
  auto * ptr = (EventSocketConnected *)arg;
  ptr->onEvent(bev, events);
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onRead(bufferevent * bev)
{
  log(DBG, "onRead");
  evbuffer * inputBuf = bufferevent_get_input(bev);
  if (!inputBuf)
  {
    log(ERR, "Can't get input");
    return;
  }

  auto bufSize = evbuffer_get_length(inputBuf);
  VecByte buf;
  buf.resize(bufSize);

  auto copied = evbuffer_remove(inputBuf, (void *)buf.data(), buf.size());
  if (copied != bufSize)
  {
    log(ERR, "Copied size: {} is not equal with buf size: {}", copied, bufSize);
    return;
  }
  log(VRB, "Incoming buffer: {}", buf);

  if (_user)
    _user->onReceive(buf);
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onWrite(bufferevent * bev)
{
  log(DBG, "Got on write");
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onEvent(bufferevent * bev, short events)
{
  log(DBG, "onEvent: {}", events);
  if (events & BEV_EVENT_EOF)
  {
    log(DBG, "Socket got EOF, close");
    closeConnection();
    if (_user) _user->onConnectionClosed();
  }

  if (events & BEV_EVENT_ERROR)
  {
    log(ERR, "Socket got error, close");
    closeConnection();
    if (_user) _user->onConnectionClosed();
  }

  if ((events & BEV_EVENT_CONNECTED) && _waitForConnect)
  {
    log(DBG, "Client connected to dest host");
    bufferevent_setcb(_bev.get(), &EventSocketConnected::onReadStatic, NULL,
                      &EventSocketConnected::onEventStatic, this);
    bufferevent_enable(_bev.get(), EV_READ | EV_WRITE);
    _localAddress = getLocalAddressImpl();
    if (_user) _user->onConnected(true);
    _connected = true;
  }
}
//-----------------------------------------------------------------------------
std::optional<SocksAddress> EventSocketConnected::getLocalAddressImpl() const
{
  SocksAddress addr;
  addr._type._value = SocksAddressType::IPv4Addr;

  sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  socklen_t namelen = sizeof(saddr);
  int res = getsockname(_fd, (sockaddr *)&saddr, &namelen);
  if (res < 0)
  {
    log(ERR, "Can't get local address: res: {}, error: {}", res, strerror(errno));
    return std::optional<SocksAddress>(std::nullopt);
  }
  SocksIPv4Address ip4Addr;
  memcpy(ip4Addr._value, &saddr.sin_addr.s_addr, namelen);
  addr._addr = ip4Addr;
  addr._port = saddr.sin_port;

  log(DBG, "Addr successfully got: {}", VecByte(std::begin(ip4Addr._value), std::end(ip4Addr._value)));
  return addr;
}
//-----------------------------------------------------------------------------
void EventSocketConnected::setUser(ISocksConnectionUser * user)
{
  _user = user;
}
//-----------------------------------------------------------------------------
bool EventSocketConnected::connect()
{
  log(DBG, "Connect");
  SocksIPv4Address & addr = std::get<SocksIPv4Address>(_peerAddress._addr);
  sockaddr_in daddr;
  daddr.sin_family = AF_INET;
  memcpy(&daddr.sin_addr.s_addr, addr._value, sizeof(daddr));
  log(DBG, "Try to connect to ip: {}, port: {}", inet_ntoa(daddr.sin_addr), htons(_peerAddress._port));
  daddr.sin_port = _peerAddress._port;
  if (bufferevent_socket_connect(_bev.get(), (sockaddr *)&daddr, sizeof(daddr)) == 0)
  {
    _waitForConnect = true;
    return true;
  }

  return false;
}
//-----------------------------------------------------------------------------
bool EventSocketConnected::send(const VecByte & buf)
{
  log(VRB, "send called: buf: {}", buf);
  evbuffer * outputBuf = bufferevent_get_output(_bev.get());
  if (evbuffer_add(outputBuf, (void *)buf.data(), buf.size()) != 0)
  {
    log(ERR, "Fail to add buffer to client");
    return false;
  }
  return true;
}
//-----------------------------------------------------------------------------
void EventSocketConnected::closeConnection()
{
  if (!_connected)
    return;

  _connected = false;
  log(DBG, "on close connection");
  bufferevent_disable(_bev.get(), EV_READ | EV_WRITE);
  evutil_closesocket(_fd);
}
//-----------------------------------------------------------------------------
std::optional<SocksAddress> EventSocketConnected::getLocalAddress() const
{
  return _localAddress;
}
//-----------------------------------------------------------------------------
