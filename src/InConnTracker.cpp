#include "InConnTracker.h"

InConnTracker::InConnTracker(uint32_t id, IConnTrackerOwner & owner, ISocksConnectionPtr conn) :
  LoggerAdapter("InConnTrack", id),
  _id(id),
  _owner(owner),
  _connection(conn),
  _machine(id, *this),
  _authRequested(false)
{
  _connection->setUser(this);
}

void InConnTracker::onProxyStarted(Byte status, const SocksAddress & localAddress)
{

}

void InConnTracker::onAuthRequestCompleted(bool success)
{

}

void InConnTracker::sendGreetingResponse(SocksAuthMethod method)
{
  if (method._value == SocksAuthMethod::AuthLoginPass)
    _authRequested = true;
}

void InConnTracker::requestPassAuth(const std::string & user, const std::string & password)
{

}

void InConnTracker::sendPassAuthResponse(Byte status)
{

}

void InConnTracker::startProxy(SocksCommandCode type, SocksAddress address)
{

}

void InConnTracker::sendCommandResponse(Byte status, const SocksAddress & localAddress)
{

}

void InConnTracker::onProtocolError(const std::string & reason)
{

}

void InConnTracker::onReceive(const VecByte & buf)
{

}

void InConnTracker::onConnected(bool connected)
{}

void InConnTracker::onConnectionClosed()
{
  _owner.onConnectionClosed(_id);
}
