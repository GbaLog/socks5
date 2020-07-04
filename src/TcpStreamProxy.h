#ifndef TcpStreamProxyH
#define TcpStreamProxyH
//-----------------------------------------------------------------------------
#include "LoggerAdapter.h"
#include "SocksInterfaces.h"
#include "DirectedProxyConnection.h"
//-----------------------------------------------------------------------------
class TcpStreamProxy : private LoggerAdapter, private IDirectedProxyConnectionOwner
{
public:
  TcpStreamProxy(uint32_t id, IProxyUser & user, ISocksConnectionPtr srcConn, ISocksConnectionPtr destConn);

private:
  IProxyUser & _user;
  DirectedProxyConnection _mainProxy;
  DirectedProxyConnection _outProxy;

  virtual void onDataReceived(ProxyDirection direction, const VecByte & buf) override;
  virtual void onConnected(ProxyDirection direction) override;
  virtual void onDisconnected(ProxyDirection direction) override;
};
//-----------------------------------------------------------------------------
#endif // TcpStreamProxyH
//-----------------------------------------------------------------------------
