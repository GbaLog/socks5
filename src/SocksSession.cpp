#include "SocksSession.h"
#include "ExceptionStream.h"
#include <algorithm>
//-----------------------------------------------------------------------------
SocksSession::SocksSession(uint32_t id, ISocksSessionUser & user, ISocksConnection & incoming, ISocksAuthorizer & auth) :
  LoggerAdapter("SockSess", id),
  _id(id),
  _user(user),
  _inConnection(incoming),
  _auth(auth),
  _outConnection(nullptr),
  _connected(false),
  _state(State::WaitForGreeting),
  _authMethod{SocksAuthMethod::NoAvailableMethod}
{}
//-----------------------------------------------------------------------------
void SocksSession::processData(const VecByte & buf)
{
  _currentMsgBuf = buf;
  process();
}
//-----------------------------------------------------------------------------
void SocksSession::clientDisconnected()
{
  disconnectAll();
}
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
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

  _authMethod = resp._authMethod;

  if (authMethod == SocksAuthMethod::NoAvailableMethod)
  {
    _inConnection.send(respBuf);
    disconnectAll();
    return;
  }

  if (_inConnection.send(respBuf) == false)
  {
    disconnectAll();
    return;
  }

  if (authMethod == SocksAuthMethod::NoAuth)
    _state = State::WaitForCommand;
  else
    _state = State::WaitForAuth;
}
//-----------------------------------------------------------------------------
void SocksSession::processAuth()
{
  switch (_authMethod._value)
  {
  case SocksAuthMethod::NoAuth:
  case SocksAuthMethod::AuthGSSAPI:
    //Not implemented yet
    disconnectAll();
    break;
  case SocksAuthMethod::AuthLoginPass:
    processAuthLoginPass();
    break;
  default:
    disconnectAll();
    break;
  }
}
//-----------------------------------------------------------------------------
void SocksSession::processCommand()
{
  auto sendCommandResWithStatus = [&] (uint8_t cmdStatus)
  {
    if (processCommandResult(cmdStatus))
      return;
    disconnectAll();
    return;
  };

  SocksCommandMsg msg;
  if (_decoder.decode(_currentMsgBuf, msg) == false)
  {
    log(DBG, "Decode");
    sendCommandResWithStatus(SocksCommandMsgResp::CommandNotSupported);
    return;
  }

  SocksAddress connectAddr;
  connectAddr._type = msg._addrType;
  connectAddr._addr = msg._addr;
  connectAddr._port = msg._port;
  _outConnection = _user.createNewConnection(*this, connectAddr);
  if (_outConnection == nullptr)
  {
    log(DBG, "ruleset");
    sendCommandResWithStatus(SocksCommandMsgResp::RulesetFailure);
    return;
  }

  if (_outConnection->connect() == false)
  {
    log(DBG, "connect");
    sendCommandResWithStatus(SocksCommandMsgResp::HostUnreachable);
    return;
  }
  log(DBG, "Wait for connect");

  _state = State::WaitForConnectResult;
}
//-----------------------------------------------------------------------------
void SocksSession::processConnectResult()
{
  auto sendCommandResWithStatus = [&] (uint8_t cmdStatus)
  {
    if (processCommandResult(cmdStatus))
      return;
    disconnectAll();
    return;
  };

  if (_connected)
    sendCommandResWithStatus(SocksCommandMsgResp::RequestGranted);
  else
    sendCommandResWithStatus(SocksCommandMsgResp::HostUnreachable);
}
//-----------------------------------------------------------------------------
void SocksSession::processConnected()
{
  if (_outConnection == nullptr)
  {
    disconnectAll();
    return;
  }

  if (_outConnection->send(_currentMsgBuf) == false)
  {
    disconnectAll();
  }
}
//-----------------------------------------------------------------------------
void SocksSession::processDisconnect()
{
  disconnectAll();
}
//-----------------------------------------------------------------------------
void SocksSession::processAuthLoginPass()
{
  SocksUserPassAuthMsg msg;
  if (_decoder.decode(_currentMsgBuf, msg) == false)
  {
    disconnectAll();
    return;
  }

  SocksUserPassAuthMsgResp resp;
  resp._version._value = SocksVersion::Version5;
  if (_auth.authUserPassword(msg._user, msg._password) == false)
  {
    resp._status = AuthStatus::Fail;
  }
  else
  {
    resp._status = AuthStatus::OK;
  }

  VecByte respBuf;
  if (_encoder.encode(resp, respBuf) == false)
  {
    disconnectAll();
    return;
  }

  if (_inConnection.send(respBuf) == false)
    disconnectAll();
  else
    _state = State::WaitForCommand;

  if (resp._status == AuthStatus::Fail)
    disconnectAll();
}
//-----------------------------------------------------------------------------
bool SocksSession::processCommandResult(uint8_t cmdStatus)
{
  SocksCommandMsgResp resp;
  resp._version._value = SocksVersion::Version5;
  resp._status = cmdStatus;

  auto nullResponseAddr = [&] ()
  {
    resp._addrType._value = SocksAddressType::IPv4Addr;
    resp._addr = SocksIPv4Address{ 0x00, 0x00, 0x00, 0x00 };
    resp._port = 0x0000;
  };

  switch (cmdStatus)
  {
  case SocksCommandMsgResp::RequestGranted:
    {
      if (_outConnection == nullptr)
      {
        log(DBG, "outconn");
        return false;
      }

      auto localOutAddr = _outConnection->getLocalAddress();
      if (localOutAddr.has_value() == false)
      {
        cmdStatus = SocksCommandMsgResp::GeneralFailure;
        nullResponseAddr();
        break;
      }
      resp._addr = localOutAddr->_addr;
      resp._addrType = localOutAddr->_type;
      resp._port = localOutAddr->_port;
      break;
    }
  case SocksCommandMsgResp::AddressNotSupported:
  case SocksCommandMsgResp::CommandNotSupported:
  case SocksCommandMsgResp::ConnectionRefused:
  case SocksCommandMsgResp::GeneralFailure:
  case SocksCommandMsgResp::HostUnreachable:
  case SocksCommandMsgResp::NetworkUnreachable:
  case SocksCommandMsgResp::RulesetFailure:
  case SocksCommandMsgResp::TTLExpired:
    nullResponseAddr();
    break;
  }

  VecByte respBuf;
  if (_encoder.encode(resp, respBuf) == false)
  {
    log(DBG, "Encode");
    return false;
  }

  if (_inConnection.send(respBuf) == false)
  {
    log(DBG, "send");
    return false;
  }

  if (cmdStatus != SocksCommandMsgResp::RequestGranted)
    return false;
  _state = State::Connected;
  return true;
}
//-----------------------------------------------------------------------------
void SocksSession::disconnectAll()
{
  if (_state == State::Disconnected)
    return;

  _inConnection.closeConnection();
  if (_outConnection != nullptr)
  {
    _outConnection->closeConnection();
    _outConnection.reset();
  }
  _state = State::Disconnected;
  _user.onConnectionDestroyed(*this, nullptr);
}
//-----------------------------------------------------------------------------
void SocksSession::onReceive(const VecByte & buf)
{
  if (_state != State::Connected)
  {
    disconnectAll();
    return;
  }

  if (_inConnection.send(buf) == false)
  {
    disconnectAll();
  }
}
//-----------------------------------------------------------------------------
void SocksSession::onConnected(bool connected)
{
  if (_state != State::WaitForConnectResult)
  {
    log(ERR, "Wrong state: should be waiting for connect result: {}", (int)_state);
    return;
  }
  _connected = connected;
  process();
}
//-----------------------------------------------------------------------------
void SocksSession::onConnectionClosed()
{
  disconnectAll();
}
