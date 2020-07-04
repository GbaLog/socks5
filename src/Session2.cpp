#include "Session2.h"

Session2::Session2(uint32_t id, ISocksSessionUser & user,
                   ISocksConnectionPtr inConn, ISocksAuthorizer & auth) :
  LoggerAdapter("Session2", id),
  _id(id),
  _user(user),
  _auth(auth),
  _inConn(inConn),
  _outConn(nullptr),
  _tracker(id, *this, inConn),
  _tcpStreamProxy(nullptr)
{}

void Session2::onProxyDestroy()
{
  destroySelf(DBG, "Proxy destroy");
}

void Session2::onStartProxy(SocksCommandCode type, SocksAddress address)
{
  if (type._value != SocksCommandCode::TCPStream)
  {
    _tracker.onProxyStarted(SocksCommandMsgResp::CommandNotSupported, SocksAddress{});
    return destroySelf(WRN, "Unsupported Command Code");
  }

  _outConn = _user.createNewConnection(*this, address);
  if (_outConn->connect() == false)
  {
    _tracker.onProxyStarted(SocksCommandMsgResp::GeneralFailure, SocksAddress{});
    return destroySelf(ERR, "Can't start connect procedure");
  }
}

void Session2::onRequestPassAuth(const std::string & user, const std::string & password)
{
  bool success = _auth.authUserPassword(user, password);
  _tracker.onAuthRequestCompleted(success);
}

void Session2::onConnTrackerDestroy()
{
  destroySelf(DBG, "Conn tracker destroy");
}

void Session2::onReceive(const VecByte & buf)
{}

void Session2::onConnected(bool connected)
{
  Byte status = SocksCommandMsgResp::RequestGranted;
  if (connected == false)
  {
    status = SocksCommandMsgResp::HostUnreachable;
    destroySelf(ERR, "Can't connect to destination");
  }

  auto localAddr = _outConn->getLocalAddress();
  if (localAddr.has_value() == false)
  {
    _tracker.onProxyStarted(SocksCommandMsgResp::HostUnreachable, SocksAddress{});
    return destroySelf(ERR, "Can't get local address of new connection");
  }
  else
  {
    _tracker.onProxyStarted(SocksCommandMsgResp::RequestGranted, *localAddr);
  }

  _tcpStreamProxy = TcpStreamProxyPtr(new TcpStreamProxy(_id, *this, _inConn, _outConn));
}

void Session2::onConnectionClosed()
{

}

void Session2::destroySelf(int level, std::string_view reason)
{
  log(level, "{}", reason);
  _user.onConnectionDestroyed(*this, nullptr);
  _user.onSessionEnd(_id);
}
