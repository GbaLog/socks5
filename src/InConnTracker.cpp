#include "InConnTracker.h"
//-----------------------------------------------------------------------------
InConnTracker::InConnTracker(uint32_t id, IConnTrackerOwner & owner, SocksConnectionPtr conn) :
  LoggerAdapter("InConnTrack", id),
  _owner(owner),
  _connection(conn),
  _machine(id, *this)
{
  _connection->setUser(this);
}
//-----------------------------------------------------------------------------
void InConnTracker::onProxyStarted(Byte status, const SocksAddress & localAddress)
{
  _machine.processStartProxyResult(status, localAddress);
}
//-----------------------------------------------------------------------------
void InConnTracker::onAuthRequestCompleted(bool success)
{
  _machine.processPassAuthResult(success);
  if (!success)
    destroySelf(WRN, "Authentication failed");
}
//-----------------------------------------------------------------------------
void InConnTracker::sendGreetingResponse(SocksAuthMethod method)
{
  SocksGreetingMsgResp msg;
  msg._authMethod = method;

  sendMsg(msg, "greeting");
}
//-----------------------------------------------------------------------------
void InConnTracker::requestPassAuth(const std::string & user, const std::string & password)
{
  _owner.onRequestPassAuth(user, password);
}
//-----------------------------------------------------------------------------
void InConnTracker::sendPassAuthResponse(Byte status)
{
  SocksUserPassAuthMsgResp msg;
  msg._status = status;

  sendMsg(msg, "pass auth");
}
//-----------------------------------------------------------------------------
void InConnTracker::startProxy(SocksCommandCode type, const SocksAddress & address)
{
  _owner.onStartProxy(type, address);
}
//-----------------------------------------------------------------------------
void InConnTracker::sendCommandResponse(Byte status, const SocksAddress & localAddress)
{
  SocksCommandMsgResp msg;
  msg._status = status;
  msg._addrType = localAddress._type;
  msg._addr = localAddress._addr;
  msg._port = localAddress._port;

  sendMsg(msg, "command");
}
//-----------------------------------------------------------------------------
void InConnTracker::onProtocolError(const std::string & reason)
{
  destroySelf(ERR, reason);
}
//-----------------------------------------------------------------------------
void InConnTracker::onReceive(const VecByte & buf)
{
  _machine.processBuffer(buf);
}
//-----------------------------------------------------------------------------
void InConnTracker::onConnected()
{}
//-----------------------------------------------------------------------------
void InConnTracker::onConnectionClosed()
{
  destroySelf(DBG, "Remote connection closed");
}
//-----------------------------------------------------------------------------
void InConnTracker::destroySelf(int level, const std::string & reason)
{
  log(level, "Destroy self, reason: {}", reason);
  _owner.onConnTrackerDestroy();
}
//-----------------------------------------------------------------------------
