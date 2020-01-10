#ifndef SocksInterfacesH
#define SocksInterfacesH

#include "SocksTypes.h"
#include <memory>

struct ISocksConnection
{
  virtual ~ISocksConnection() = default;
  virtual bool connect() = 0;
  virtual bool send(const VecByte & buf) = 0;
  virtual void closeConnection() = 0;
  virtual SocksAddress getLocalAddress() const = 0;
};
typedef std::shared_ptr<ISocksConnection> ISocksConnectionPtr;

struct ISocksConnectionUser
{
  virtual ~ISocksConnectionUser() = default;
  virtual void onReceive(const VecByte & buf) = 0;
  virtual void onConnectionClosed() = 0;
};

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

#endif // SocksInterfacesH
