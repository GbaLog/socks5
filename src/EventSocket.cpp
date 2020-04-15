#include "EventSocket.h"

EventSocket::EventSocket(EventBasePtr base, evutil_socket_t fd) :
  Traceable("EventSock", std::to_string(fd)),
  _base(base),
  _fd(fd),
  _bev(bufferevent_socket_new(_base.get(), _fd, BEV_OPT_CLOSE_ON_FREE),
       bufferevent_free),
  _user(nullptr)
{
  bufferevent_setcb(_bev.get(), &EventSocket::onReadStatic, &EventSocket::onWriteStatic,
                    &EventSocket::onEventStatic, this);
  bufferevent_enable(_bev.get(), EV_READ | EV_WRITE);
}

void EventSocket::onReadStatic(bufferevent * bev, void * arg)
{
  auto * ptr = (EventSocket *)arg;
  ptr->onRead(bev);
}

void EventSocket::onWriteStatic(bufferevent * bev, void * arg)
{
  auto * ptr = (EventSocket *)arg;
  ptr->onWrite(bev);
}

void EventSocket::onEventStatic(bufferevent * bev, short events, void * arg)
{
  auto * ptr = (EventSocket *)arg;
  ptr->onEvent(bev, events);
}

void EventSocket::onRead(bufferevent * bev)
{
  TRACE(DBG) << "onRead";
  evbuffer * inputBuf = bufferevent_get_input(bev);
  if (!inputBuf)
  {
    TRACE(ERR) << "Can't get input";
    return;
  }

  auto bufSize = evbuffer_get_length(inputBuf);
  VecByte buf;
  buf.resize(bufSize);

  auto copied = evbuffer_remove(inputBuf, (void *)buf.data(), buf.size());
  if (copied != bufSize)
  {
    TRACE(ERR) << "Copied size: " << copied << " is not equal with buf size: " << bufSize;
    return;
  }
  TRACE(DBG) << "Incoming buffer: " << buf;

  if (_user)
    _user->onReceive(buf);
}

void EventSocket::onWrite(bufferevent * bev)
{
  TRACE(DBG) << "Got on write";
}

void EventSocket::onEvent(bufferevent * bev, short events)
{
  TRACE(DBG) << "onEvent: " << events;
  if (events & BEV_EVENT_EOF)
  {
    TRACE(DBG) << "Socket got EOF, close";
    bufferevent_free(bev);
    if (_user) _user->onConnectionClosed();
  }

  if (events & BEV_EVENT_ERROR)
  {
    TRACE(ERR) << "Socket got error, close";
    bufferevent_free(bev);
    if (_user) _user->onConnectionClosed();
  }
}

void EventSocket::setUser(ISocksConnectionUser * user)
{
  _user = user;
}

bool EventSocket::connect()
{
  //Not implemented
  return false;
}

bool EventSocket::send(const VecByte & buf)
{
  TRACE(DBG) << "send called: buf: " << buf;
  evbuffer * outputBuf = bufferevent_get_output(_bev.get());
  if (evbuffer_add(outputBuf, (void *)buf.data(), buf.size()) != 0)
  {
    TRACE(ERR) << "Fail to add buffer to client";
    return false;
  }
  return true;
}

void EventSocket::closeConnection()
{
  TRACE(DBG) << "on close connection";
  bufferevent_disable(_bev.get(), EV_READ | EV_WRITE);
  evutil_closesocket(_fd);
}

std::optional<SocksAddress> EventSocket::getLocalAddress() const
{
  TRACE(DBG) << "get local address";
  return std::optional<SocksAddress>(std::nullopt);
}
