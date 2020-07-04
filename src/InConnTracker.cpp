#include "InConnTracker.h"
//-----------------------------------------------------------------------------
InConnTracker::InConnTracker(uint32_t id, IConnTrackerOwner & owner, ISocksConnectionPtr conn) :
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

  sendMsg(msg);
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

  sendMsg(msg);
}
//-----------------------------------------------------------------------------
void InConnTracker::startProxy(SocksCommandCode type, SocksAddress address)
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

  sendMsg(msg);
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
void InConnTracker::onConnected(bool connected)
{}
//-----------------------------------------------------------------------------
void InConnTracker::onConnectionClosed()
{
  destroySelf(DBG, "Remote connection closed");
}
//-----------------------------------------------------------------------------
void InConnTracker::destroySelf(int level, std::string_view reason)
{
  log(level, "Destroy self, reason: {}", reason.data());
  _owner.onConnTrackerDestroy();
}
//-----------------------------------------------------------------------------
