#ifndef StateMachineH
#define StateMachineH
//-----------------------------------------------------------------------------
#include "SocksTypes.h"
//-----------------------------------------------------------------------------
class IStateMachineOwner
{
public:
  virtual ~IStateMachineOwner() = default;

  virtual void sendGreetingResponse(SocksAuthMethod method) = 0;
  virtual void requestPassAuth(const std::string & user, const std::string & password) = 0;
  virtual void sendPassAuthResponse(Byte status) = 0;
  virtual void startProxy(SocksCommandCode type, SocksAddress address) = 0;
  virtual void sendCommandResponse(Byte status, const SocksAddress & localAddress) = 0;
  virtual void onProtocolError(std::string_view reason) = 0;
};
//-----------------------------------------------------------------------------
class StateMachine
{
public:
  StateMachine(IStateMachineOwner & owner);

  void processGreetingMsg(const SocksGreetingMsg & msg);
  void processPassAuthMsg(const SocksUserPassAuthMsg & msg);
  void processCommandMsg(const SocksCommandMsg & msg);

  void processPassAuthResult(bool success);
  void processStartProxyResult(Byte status, const SocksAddress & localAddress);

private:
  IStateMachineOwner & _owner;

  enum class State
  {
    WaitForGreeting,
    WaitForPassAuth,
    WaitForCommand,
    WaitForProxyStart,
    ProxyStarted,
    ProxyStartFailed,
    ProtocolError
  };
  State _state;
  std::string_view stateToStr(State state) noexcept;
  void setState(State newState);

  void protocolError(std::string_view reason);
};
//-----------------------------------------------------------------------------
#endif // StateMachineH
//-----------------------------------------------------------------------------
