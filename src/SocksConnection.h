#ifndef SocksConnectionH
#define SocksConnectionH

#include "SocksInterfaces.h"

class SocksConnection : public ISocksConnection
{
public:
  SocksConnection(ISocksConnectionUser & user, const SocksAddress & localAddr, const SocksAddress & remoteAddr);

private:
  ISocksConnectionUser & _user;

  //ISocksConnections
  virtual bool connect() override;
  virtual bool send(const VecByte & buf) override;
  virtual void closeConnection() override;
};

#endif // SocksConnectionH
