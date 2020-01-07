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
  virtual ISocksConnectionPtr createNewConnection(SocksAddressType addrType, const SocksAddress & addr) = 0;
};

struct ISocksAuthorizer
{
  virtual ~ISocksAuthorizer() = default;
  virtual bool authUserPassword(const std::string & user, const std::string & password) = 0;
};

#endif // SocksInterfacesH
