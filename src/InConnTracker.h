#ifndef InConnTrackerH
#define InConnTrackerH

#include "StateMachine.h"
#include "LoggerAdapter.h"
#include "SocksInterfaces.h"

class IConnTrackerOwner
{
public:
  virtual ~IConnTrackerOwner() = default;

  virtual void onStartProxy(uint32_t id, SocksCommandCode type, SocksAddress address) = 0;
  virtual void onRequestPassAuth(uint32_t id, const std::string & user, const std::string & password) = 0;
  virtual void onConnectionClosed(uint32_t id) = 0;
};

class InConnTracker final : private IStateMachineOwner, private LoggerAdapter,
                            private ISocksConnectionUser
{
public:
  InConnTracker(uint32_t id, IConnTrackerOwner & owner, ISocksConnectionPtr conn);

  void onProxyStarted(Byte status, const SocksAddress & localAddress);
  void onAuthRequestCompleted(bool success);

private:
  const uint32_t _id;
  IConnTrackerOwner & _owner;
  ISocksConnectionPtr _connection;
  StateMachine _machine;
  bool _authRequested;

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
};

#endif // InConnTrackerH