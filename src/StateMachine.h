#ifndef StateMachineH
#define StateMachineH
//-----------------------------------------------------------------------------
#include "SocksTypes.h"
#include "LoggerAdapter.h"
#include "SocksDecoder.h"
//-----------------------------------------------------------------------------
class IStateMachineOwner
{
public:
  virtual ~IStateMachineOwner() = default;

  virtual void sendGreetingResponse(SocksAuthMethod method) = 0;
  virtual void requestPassAuth(const std::string & user, const std::string & password) = 0;
  virtual void sendPassAuthResponse(Byte status) = 0;
  virtual void startProxy(SocksCommandCode type, const SocksAddress & address) = 0;
  virtual void sendCommandResponse(Byte status, const SocksAddress & localAddress) = 0;
  virtual void onProtocolError(const std::string & reason) = 0;
};
//-----------------------------------------------------------------------------
class StateMachine : private LoggerAdapter
{
public:
  StateMachine(uint32_t id, IStateMachineOwner & owner);

  void processBuffer(const VecByte & buffer);
  void processPassAuthResult(bool success);
  void processStartProxyResult(Byte status, const SocksAddress & localAddress);

private:
  IStateMachineOwner & _owner;
  SocksDecoder _decoder;

  enum class State
  {
    WaitForGreeting,
    WaitForPassAuth,
    WaitForPassAuthResult,
    WaitForCommand,
    WaitForProxyStart,
    ProxyStarted,
    ProxyStartFailed,
    ProtocolError
  };
  State _state;
  std::string_view stateToStr(State state) noexcept;
  void setState(State newState);

  void processGreetingMsg(const SocksGreetingMsg & msg);
  void processPassAuthMsg(const SocksUserPassAuthMsg & msg);
  void processCommandMsg(const SocksCommandMsg & msg);
  void protocolError(std::string_view reason);
};
//-----------------------------------------------------------------------------
#endif // StateMachineH
//-----------------------------------------------------------------------------
