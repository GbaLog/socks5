#ifndef InConnTrackerH
#define InConnTrackerH
//-----------------------------------------------------------------------------
#include "StateMachine.h"
#include "LoggerAdapter.h"
#include "SocksInterfaces.h"
#include "SocksEncoder.h"
//-----------------------------------------------------------------------------
class InConnTracker final : private IStateMachineOwner, private LoggerAdapter,
                            private ISocksConnectionUser
{
public:
  InConnTracker(uint32_t id, IConnTrackerOwner & owner, ISocksConnectionPtr conn);

  void onProxyStarted(Byte status, const SocksAddress & localAddress);
  void onAuthRequestCompleted(bool success);

private:
  IConnTrackerOwner & _owner;
  ISocksConnectionPtr _connection;
  StateMachine _machine;
  SocksEncoder _encoder;

  // IStateMachineOwner
  virtual void sendGreetingResponse(SocksAuthMethod method) override;
  virtual void requestPassAuth(const std::string & user, const std::string & password) override;
  virtual void sendPassAuthResponse(Byte status) override;
  virtual void startProxy(SocksCommandCode type, SocksAddress address) override;
  virtual void sendCommandResponse(Byte status, const SocksAddress & localAddress) override;
  virtual void onProtocolError(const std::string & reason) override;

  // ISocksConnectionUser
  virtual void onReceive(const VecByte & buf) override;
  virtual void onConnected(bool connected) override;
  virtual void onConnectionClosed() override;

  void destroySelf(int level, std::string_view reason);

  template<class T>
  void sendMsg(const T & msg)
  {
    VecByte buf;
    if (_encoder.encode(msg, buf) == false)
    {
      destroySelf(ERR, "Can't encode msg");
      return;
    }

    if (_connection->send(buf) == false)
    {
      destroySelf(ERR, "Can't send buffer for msg");
    }
  }
};
//-----------------------------------------------------------------------------
#endif // InConnTrackerH
//-----------------------------------------------------------------------------
