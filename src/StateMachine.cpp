#include "StateMachine.h"
#include <algorithm>

StateMachine::StateMachine(IStateMachineOwner & owner) :
  _owner(owner)
{}

void StateMachine::processGreetingMsg(const SocksGreetingMsg & msg)
{
  if (_state != State::WaitForGreeting)
    return protocolError("Wait for greeting expected");

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

}

void StateMachine::processCommandMsg(const SocksCommandMsg & msg)
{

}

void StateMachine::processPassAuthResult(bool success)
{

}

void StateMachine::processStartProxyResult(Byte status, const SocksAddress & localAddress)
{

}

std::string_view StateMachine::stateToStr(StateMachine::State state) noexcept
{
  switch (state)
  {
#define CASE(s) case s: return #s
    CASE(State::WaitForGreeting);
    CASE(State::WaitForPassAuth);
    CASE(State::WaitForCommand);
    CASE(State::WaitForProxyStart);
    CASE(State::ProxyStarted);
    CASE(State::ProxyStartFailed);
#undef CASE
  default:
    return "State::Unknown";
  }
}

void StateMachine::setState(StateMachine::State newState)
{

}

void StateMachine::protocolError(std::string_view reason)
{
  setState(State::ProtocolError);
  _owner.onProtocolError(reason);
}
