#include "SocksSession.h"
#include "ratellib/ExceptionStream.h"
#include <algorithm>

SocksSession::SocksSession(uint32_t id, ISocksSessionUser & user, ISocksConnection & incoming, ISocksAuthorizer & auth) :
  Traceable("SockSess", id),
  _id(id),
  _user(user),
  _inConnection(incoming),
  _auth(auth),
  _decoder({ SocksVersion::Version5 }),
  _encoder({ SocksVersion::Version5 }),
  _outConnection(nullptr),
  _state(State::WaitForGreeting)
{}

void SocksSession::processData(const VecByte & buf)
{
  _currentMsgBuf = buf;
  process();
}

void SocksSession::clientDisconnected()
{
  _state = State::Disconnected;
  process();
}

void SocksSession::process()
{
  switch (_state)
  {
  case State::WaitForGreeting:
    processGreeting();
    break;
  case State::WaitForAuth:
    processAuth();
    break;
  case State::WaitForCommand:
    processCommand();
    break;
  case State::WaitForConnectResult:
    processConnectResult();
    break;
  case State::Connected:
    processConnected();
    break;
  case State::Disconnected:
    processDisconnect();
    break;
  default:
    RATEL_THROW(std::runtime_error) << "Unknown state: " << (int)_state;
  }
}

void SocksSession::processGreeting()
{
  SocksGreetingMsg msg;
  if (_decoder.decode(_currentMsgBuf, msg) == false)
  {
    disconnectAll();
    return;
  }

  auto canUseMethod = [&] (SocksAuthMethod method)
  {
    return std::find(msg._authMethods.begin(), msg._authMethods.end(),
                     method) != msg._authMethods.end(); //&&
//           _auth.isMethodSupported(method);
  };

  SocksGreetingMsgResp resp;
  resp._version._value = SocksVersion::Version5;

  auto & authMethod = resp._authMethod._value;
  //Priority in auth methods:
  // First priority is GSSAPI, because it's the safest method,
  // but it's not supported yet.
  // Next is the Login/Password auth method
  // And last and unsafe method is when we don't any
  // All except Login/Password are disabled for now

//  if (canUseMethod({ SocksAuthMethod::AuthGSSAPI }))
//    authMethod = SocksAuthMethod::AuthGSSAPI;
//  else
  if (canUseMethod({ SocksAuthMethod::AuthLoginPass }))
    authMethod = SocksAuthMethod::AuthLoginPass;
//  else if (canUseMethod({ SocksAuthMethod::NoAuth }))
//    authMethod = SocksAuthMethod::NoAuth;
  else
    authMethod = SocksAuthMethod::NoAvailableMethod;

  VecByte respBuf;
  if (_encoder.encode(resp, respBuf) == false)
  {
    disconnectAll();
    return;
  }

  if (_inConnection.send(respBuf) == false)
    disconnectAll();
  else if (authMethod == SocksAuthMethod::NoAuth)
    _state = State::WaitForCommand;
  else
    _state = State::WaitForAuth;
}

void SocksSession::processAuth()
{

}

void SocksSession::processCommand()
{

}

void SocksSession::processConnectResult()
{

}

void SocksSession::processConnected()
{

}

void SocksSession::processDisconnect()
{

}

void SocksSession::disconnectAll()
{
  _inConnection.closeConnection();
  if (_outConnection != nullptr)
  {
    _outConnection->closeConnection();
    _outConnection.reset();
  }
  _state = State::Disconnected;
  _user.onConnectionDestroyed(*this, nullptr);
}

void SocksSession::onReceive(const VecByte & buf)
{

}

void SocksSession::onConnectionClosed()
{

}
