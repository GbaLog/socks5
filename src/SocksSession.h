#ifndef SocksSessionH
#define SocksSessionH

#include "SocksInterfaces.h"
#include "SocksDecoder.h"

class SocksSession : public ISocksConnectionUser
{
public:
  SocksSession(ISocksSessionUser & user, ISocksConnection & incoming, ISocksAuthorizer & auth);

  void processData(const VecByte & buf);
  void clientDisconnected();

private:
  ISocksSessionUser & _user;
  ISocksConnection & _inConnection;
  ISocksAuthorizer & _auth;
  SocksDecoder _decoder;

  virtual void onReceive(const VecByte & buf) override;
  virtual void onConnectionClosed() override;
};

#endif // SocksSessionH
