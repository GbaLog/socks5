#include "TcpStreamProxy.h"
//-----------------------------------------------------------------------------
TcpStreamProxy::TcpStreamProxy(uint32_t id, IProxyUser & user,
                               SocksConnectionPtr srcConn, SocksConnectionPtr destConn) :
  LoggerAdapter("TcpStreamPx", id),
  _user(user),
  _mainProxy(id, *this, ProxyDirection::Main, srcConn),
  _outProxy(id, *this, ProxyDirection::Out, destConn)
{}
//-----------------------------------------------------------------------------
void TcpStreamProxy::onDataReceived(ProxyDirection direction, const VecByte & buf)
{
  switch (direction)
  {
  case ProxyDirection::Main:
    log(DBG, "Buffer received from main: len: {}", buf.size());
    _outProxy.send(buf);
    break;
  case ProxyDirection::Out:
    log(DBG, "Buffer received from out: len: {}", buf.size());
    _mainProxy.send(buf);
    break;
  default:
    break;
  }
}
//-----------------------------------------------------------------------------
void TcpStreamProxy::onConnected(ProxyDirection direction)
{}
//-----------------------------------------------------------------------------
void TcpStreamProxy::onDisconnected(ProxyDirection direction)
{
  switch (direction)
  {
  case ProxyDirection::Main:
    _outProxy.disconnect();
    break;
  case ProxyDirection::Out:
    _mainProxy.disconnect();
    break;
  default:
    break;
  }

  _user.onProxyDestroy();
}
//-----------------------------------------------------------------------------
