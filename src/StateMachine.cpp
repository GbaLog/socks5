#include "StateMachine.h"
#include <algorithm>

StateMachine::StateMachine(uint32_t id, IStateMachineOwner & owner) :
  LoggerAdapter("StateMachine", id),
  _owner(owner),
  _state(State::WaitForGreeting)
{}

void StateMachine::processBuffer(const VecByte & buffer)
{
  switch (_state)
  {
  case State::WaitForGreeting:
    {
      SocksGreetingMsg msg;
      if (_decoder.decode(buffer, msg) == false)
        return protocolError("Greeting decode error");
      processGreetingMsg(msg);
      break;
    }
  case State::WaitForPassAuth:
    {
      SocksUserPassAuthMsg msg;
      if (_decoder.decode(buffer, msg) == false)
        return protocolError("User/Password authentication decode error");
      processPassAuthMsg(msg);
      break;
    }
  case State::WaitForCommand:
    {
      SocksCommandMsg msg;
      if (_decoder.decode(buffer, msg) == false)
        return protocolError("Command decode error");
      processCommandMsg(msg);
      break;
    }
  default:
    protocolError("Message in wrong state");
    break;
  }
}

void StateMachine::processGreetingMsg(const SocksGreetingMsg & msg)
{
  if (_state != State::WaitForGreeting)
    return protocolError("Greeting is not expected");

  const auto & authMethods = msg._authMethods;

  auto res = SocksAuthMethod{SocksAuthMethod::NoAvailableMethod};
  auto it = std::find(authMethods.begin(), authMethods.end(),
                      SocksAuthMethod{ SocksAuthMethod::AuthLoginPass });
  if (it != authMethods.end())
    res._value = SocksAuthMethod::AuthLoginPass;
  _owner.sendGreetingResponse(res);

  if (res._value == SocksAuthMethod::AuthLoginPass)
    setState(State::WaitForPassAuth);
  else
    protocolError("Unsupported auth method");
}

void StateMachine::processPassAuthMsg(const SocksUserPassAuthMsg & msg)
{
  if (_state != State::WaitForPassAuth)
    return protocolError("Password auth is not expected");

  setState(State::WaitForPassAuthResult);
  _owner.requestPassAuth(msg._user, msg._password);
}

void StateMachine::processCommandMsg(const SocksCommandMsg & msg)
{
  if (_state != State::WaitForCommand)
    return protocolError("Command is not expected");

  setState(State::WaitForProxyStart);

  SocksAddress addr;
  addr._type = msg._addrType;
  addr._addr = msg._addr;
  addr._port = msg._port;
  _owner.startProxy(msg._command, addr);
}

void StateMachine::processPassAuthResult(bool success)
{
  if (_state != State::WaitForPassAuthResult)
    return protocolError("Password authentication result is not expected");

  if (success)
  {
    setState(State::WaitForCommand);
    _owner.sendPassAuthResponse(0x00);
  }
  else
  {
    _owner.sendPassAuthResponse(0x01);
  }
}

void StateMachine::processStartProxyResult(Byte status, const SocksAddress & localAddress)
{
  if (_state != State::WaitForProxyStart)
    return protocolError("Start proxy is not expected");

  _owner.sendCommandResponse(status, localAddress);
  if (status == SocksCommandMsgResp::RequestGranted)
  {
    setState(State::ProxyStarted);
  }
  else
  {
    setState(State::ProxyStartFailed);
  }
}

std::string_view StateMachine::stateToStr(StateMachine::State state) noexcept
{
  switch (state)
  {
#define CASE(s) case s: return #s
  CASE(State::WaitForGreeting);
  CASE(State::WaitForPassAuth);
  CASE(State::WaitForPassAuthResult);
  CASE(State::WaitForCommand);
  CASE(State::WaitForProxyStart);
  CASE(State::ProxyStarted);
  CASE(State::ProxyStartFailed);
  CASE(State::ProtocolError);
#undef CASE
  default:
    return "State::Unknown";
  }
}

void StateMachine::setState(State newState)
{
  log(VRB, "Changing state from: {} to: {}", stateToStr(_state), stateToStr(newState));
  _state = newState;
}

void StateMachine::protocolError(std::string_view reason)
{
  setState(State::ProtocolError);

  std::ostringstream strm;
  strm << "Protocol error in state: " << stateToStr(_state) << ", reason: " << reason;
  std::string errStr = strm.str();
  log(ERR, "{}", errStr);
  _owner.onProtocolError(std::move(errStr));
}
