#ifndef SocksInterfacesH
#define SocksInterfacesH
//-----------------------------------------------------------------------------
#include "SocksTypes.h"
#include <memory>
#include <optional>
//-----------------------------------------------------------------------------
struct ISocksConnectionUser
{
  virtual ~ISocksConnectionUser() = default;
  virtual void onReceive(const VecByte & buf) = 0;
  virtual void onConnected(bool connected) = 0;
  virtual void onConnectionClosed() = 0;
};
//-----------------------------------------------------------------------------
struct ISocksConnection
{
  virtual ~ISocksConnection() = default;
  virtual void setUser(ISocksConnectionUser * user) = 0;
  virtual bool connect() = 0;
  virtual bool send(const VecByte & buf) = 0;
  virtual void closeConnection() = 0;
  virtual bool isConnected() const = 0;
  virtual std::optional<SocksAddress> getLocalAddress() const = 0;
};
//-----------------------------------------------------------------------------
typedef std::shared_ptr<ISocksConnection> SocksConnectionPtr;
//-----------------------------------------------------------------------------
struct ISocksSessionUser
{
  virtual ~ISocksSessionUser() = default;
  virtual SocksConnectionPtr createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr) = 0;
  virtual void onConnectionDestroyed(ISocksConnectionUser & user, SocksConnectionPtr conn) = 0;
  virtual void onSessionEnd(uint32_t id) = 0;
};
//-----------------------------------------------------------------------------
struct ISocksAuthorizer
{
  virtual ~ISocksAuthorizer() = default;
  virtual bool isMethodSupported(const SocksAuthMethod & method) const = 0;
  virtual bool authUserPassword(const std::string & user, const std::string & password) const = 0;
};
//-----------------------------------------------------------------------------
struct ITcpServerUser
{
  virtual ~ITcpServerUser() = default;
  virtual void onNewConnection(ISocksConnection * newConn) = 0;
};
//-----------------------------------------------------------------------------
class IConnTrackerOwner
{
public:
  virtual ~IConnTrackerOwner() = default;

  virtual void onStartProxy(SocksCommandCode type, SocksAddress address) = 0;
  virtual void onRequestPassAuth(const std::string & user, const std::string & password) = 0;
  virtual void onConnTrackerDestroy() = 0;
};
//-----------------------------------------------------------------------------
enum class ProxyDirection
{
  Main, ///< Original incoming connection
  In,   ///< TCP/UDP server, TCP socket
  Out   ///< TCP connection(s)
};
//-----------------------------------------------------------------------------
class IDirectedProxyConnectionOwner
{
public:
  virtual ~IDirectedProxyConnectionOwner() = default;

  virtual void onDataReceived(ProxyDirection direction, const VecByte & buf) = 0;
  virtual void onConnected(ProxyDirection direction) = 0;
  virtual void onDisconnected(ProxyDirection direction) = 0;
};
//-----------------------------------------------------------------------------
class IProxyUser
{
public:
  virtual ~IProxyUser() = default;

  virtual void onProxyDestroy() = 0;
};
//-----------------------------------------------------------------------------
#endif // SocksInterfacesH
//-----------------------------------------------------------------------------
