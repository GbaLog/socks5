#ifndef SocksSessionH
#define SocksSessionH

#include "SocksInterfaces.h"
#include "SocksDecoder.h"
#include "SocksEncoder.h"
#include "Tracer.h"

class SocksSession : public ISocksConnectionUser, private Traceable
{
public:
  SocksSession(uint32_t id, ISocksSessionUser & user, ISocksConnection & incoming, ISocksAuthorizer & auth);

  void processData(const VecByte & buf);
  void clientDisconnected();

private:
  const uint32_t _id;
  ISocksSessionUser & _user;
  ISocksConnection & _inConnection;
  ISocksAuthorizer & _auth;
  SocksDecoder _decoder;
  SocksEncoder _encoder;
  ISocksConnectionPtr _outConnection;

  enum class State : int
  {
    WaitForGreeting,
    WaitForAuth,
    WaitForCommand,
    WaitForConnectResult,
    Connected,
    Disconnected
  };

  State _state;
  VecByte _currentMsgBuf;

  void process();
  void processGreeting();
  void processAuth();
  void processCommand();
  void processConnectResult();
  void processConnected();
  void processDisconnect();

  void disconnectAll();

  virtual void onReceive(const VecByte & buf) override;
  virtual void onConnectionClosed() override;
};

#endif // SocksSessionH
