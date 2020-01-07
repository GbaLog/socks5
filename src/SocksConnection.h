#ifndef SocksConnectionH
#define SocksConnectionH

#include "SocksInterfaces.h"

class SocksConnection : public ISocksConnection
{
public:
  SocksConnection(SocksAddressType addrType, const SocksAddress & addr);

private:

  //ISocksConnections
  virtual bool connect() override;
  virtual bool send(const VecByte & buf) override;
  virtual void closeConnection() override;
};

#endif // SocksConnectionH
