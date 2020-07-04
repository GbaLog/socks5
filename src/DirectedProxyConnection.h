#ifndef DirectedProxyConnectionH
#define DirectedProxyConnectionH
//-----------------------------------------------------------------------------
#include "SocksInterfaces.h"
#include "LoggerAdapter.h"
//-----------------------------------------------------------------------------
class DirectedProxyConnection : private LoggerAdapter, private ISocksConnectionUser
{
public:
  DirectedProxyConnection(uint32_t id, IDirectedProxyConnectionOwner & owner,
                          ProxyDirection direction, SocksConnectionPtr conn);
  ~DirectedProxyConnection();

  bool send(const VecByte & buf);
  void disconnect();

private:
  IDirectedProxyConnectionOwner & _owner;
  ProxyDirection _direction;
  SocksConnectionPtr _connection;
  bool _isConnected;

  // ISocksConnectionUser
  virtual void onReceive(const VecByte & buf) override;
  virtual void onConnected(bool connected) override;
  virtual void onConnectionClosed() override;

  static std::string formatLoggerId(uint32_t id, ProxyDirection direction);
};
//-----------------------------------------------------------------------------
#endif // DirectedProxyConnectionH
//-----------------------------------------------------------------------------
