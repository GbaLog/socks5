#ifndef SocksSessionH
#define SocksSessionH
//-----------------------------------------------------------------------------
#include "SocksInterfaces.h"
#include "SocksDecoder.h"
#include "SocksEncoder.h"
#include "LoggerAdapter.h"
//-----------------------------------------------------------------------------
class SocksSession : public ISocksConnectionUser, private LoggerAdapter
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
  bool _connected;

  enum class State : int
  {
    WaitForGreeting,
    WaitForAuth,
    WaitForCommand,
    WaitForConnectResult,
    Connected,
    Disconnected
  };

  enum AuthStatus : Byte
  {
    OK = 0x00,
    Fail = 0x01
  };

  State _state;
  VecByte _currentMsgBuf;
  SocksAuthMethod _authMethod;

  void process();
  void processGreeting();
  void processAuth();
  void processCommand();
  void processConnectResult();
  void processConnected();
  void processDisconnect();

  void processAuthLoginPass();
  bool processCommandResult(uint8_t cmdStatus);

  void disconnectAll();

  virtual void onReceive(const VecByte & buf) override;
  virtual void onConnected(bool connected) override;
  virtual void onConnectionClosed() override;
};
//-----------------------------------------------------------------------------
#endif // SocksSessionH
//-----------------------------------------------------------------------------
