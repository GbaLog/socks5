#ifndef SocksInterfacesH
#define SocksInterfacesH

#include "SocksTypes.h"
#include <memory>
#include <optional>

struct ISocksConnectionUser
{
  virtual ~ISocksConnectionUser() = default;
  virtual void onReceive(const VecByte & buf) = 0;
  virtual void onConnected(bool connected) = 0;
  virtual void onConnectionClosed() = 0;
};

struct ISocksConnection
{
  virtual ~ISocksConnection() = default;
  virtual void setUser(ISocksConnectionUser * user) = 0;
  virtual bool connect() = 0;
  virtual bool send(const VecByte & buf) = 0;
  virtual void closeConnection() = 0;
  virtual std::optional<SocksAddress> getLocalAddress() const = 0;
};
typedef std::shared_ptr<ISocksConnection> ISocksConnectionPtr;

struct ISocksSessionUser
{
  virtual ~ISocksSessionUser() = default;
  virtual ISocksConnectionPtr createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr) = 0;
  virtual void onConnectionDestroyed(ISocksConnectionUser & user, ISocksConnectionPtr conn) = 0;
};

struct ISocksAuthorizer
{
  virtual ~ISocksAuthorizer() = default;
  virtual bool isMethodSupported(const SocksAuthMethod & method) const = 0;
  virtual bool authUserPassword(const std::string & user, const std::string & password) const = 0;
};

struct ITcpServerUser
{
  virtual ~ITcpServerUser() = default;
  virtual void onNewConnection(ISocksConnection * newConn) = 0;
};

#endif // SocksInterfacesH
