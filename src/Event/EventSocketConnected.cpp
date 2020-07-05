#include "EventSocketConnected.h"
#include <cstring>
//-----------------------------------------------------------------------------
EventSocketConnected::EventSocketConnected(EventBasePtr base, evutil_socket_t fd) :
  LoggerAdapter("EventSock", fmt::format("{}", fd)),
  _bev(makeBufferEvent(base, fd)),
  _user(nullptr),
  _connected(false)
{
  enable();
}
//-----------------------------------------------------------------------------
SocksConnectionPtr EventSocketConnected::createConnect(EventBasePtr base,
                                                       const SocksAddress & addr, ISocksConnectionUser * user)
{
  auto sock = SocksConnectionPtr(new EventSocketConnected(base, addr));
  sock->setUser(user);
  if (sock->connect() == false)
    return nullptr;
  return sock;
}
//-----------------------------------------------------------------------------
EventSocketConnected::EventSocketConnected(EventBasePtr base, const SocksAddress & addr) :
  LoggerAdapter("EvSockConn"),
  _bev(makeBufferEvent(base, -1)),
  _user(nullptr),
  _remoteAddress(addr),
  _connected(false)
{
  enable();
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
  log(VRB, "onRead");
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
  log(VRB, "Got on write");
}
//-----------------------------------------------------------------------------
void EventSocketConnected::onEvent(bufferevent * bev, short events)
{
  log(VRB, "Got event: {}", events);
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

  if (events & BEV_EVENT_CONNECTED)
  {
    log(DBG, "Client connected to host");
    updateLocalAddress();
    updateRemoteAddress();

    _connected = true;
    if (_user)
    {
      _user->onConnected();
    }
  }
}
//-----------------------------------------------------------------------------
void EventSocketConnected::doShutdown()
{
#ifdef _WIN32
  shutdown(bufferevent_getfd(_bev.get()), SD_SEND);
#else
  shutdown(bufferevent_getfd(_bev.get()), SHUT_WR);
#endif
}
//-----------------------------------------------------------------------------
void EventSocketConnected::updateLocalAddress()
{
  _localAddress = getLocalSocketAddress(bufferevent_getfd(_bev.get()));
}
//-----------------------------------------------------------------------------
void EventSocketConnected::updateRemoteAddress()
{
  _remoteAddress = getRemoteSocketAddress(bufferevent_getfd(_bev.get()));
}
//-----------------------------------------------------------------------------
void EventSocketConnected::enable()
{
  bufferevent_setcb(_bev.get(), onReadStatic, onWriteStatic, onEventStatic, this);
  bufferevent_enable(_bev.get(), EV_READ | EV_WRITE);
}
//-----------------------------------------------------------------------------
bool EventSocketConnected::connect()
{
  log(VRB, "Connect called");

  sockaddr_storage sa;
  if (convertAddrToStorage(_remoteAddress, &sa) == false)
  {
    log(ERR, "Cannot convert address to storage!");
    return false;
  }

  log(DBG, "Try to connect to ip: {}, port: {}",
      getAddrStr((const sockaddr *)&sa), getAddrPort((const sockaddr *)&sa));
  if (bufferevent_socket_connect(_bev.get(), (const sockaddr *)&sa, sizeof(sa)) == 0)
  {
    return true;
  }

  return false;
}
//-----------------------------------------------------------------------------
void EventSocketConnected::setUser(ISocksConnectionUser * user)
{
  _user = user;
}
//-----------------------------------------------------------------------------
bool EventSocketConnected::send(const VecByte & buf)
{
  log(VRB, "Send called: buf: {}", buf);
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
  log(DBG, "Close connection");
  bufferevent_disable(_bev.get(), EV_READ | EV_WRITE);
  evutil_closesocket(bufferevent_getfd(_bev.get()));
}
//-----------------------------------------------------------------------------
bool EventSocketConnected::isConnected() const
{
  return _connected;
}
//-----------------------------------------------------------------------------
std::optional<SocksAddress> EventSocketConnected::getLocalAddress() const
{
  return _localAddress;
}
//-----------------------------------------------------------------------------
