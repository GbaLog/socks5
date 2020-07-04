#include "SocksSessionMng.h"

SocksSessionMng::SocksSessionMng(sockaddr_in addr) :
  LoggerAdapter("SockSessMng"),
  _server(*this, addr),
  _currentId(0),
  _authorizer("logins.txt")
{}

int SocksSessionMng::run()
{
  return _server.run();
}

void SocksSessionMng::onNewConnection(ISocksConnection * newConn)
{
  log(DBG, "create new connection");
  auto id = _currentId++;
  auto newSess = std::make_unique<SocksSession>(id, *this, *newConn, _authorizer);
  SessionParams params;
  params._id = id;
  params._inConn = newConn;
  params._outConn = nullptr;
  params._session = std::move(newSess);

  auto ptr = std::make_unique<SessionParams>(std::move(params));
  ptr->_inConn->setUser(ptr.get());
  _sessions[id] = std::move(ptr);
}

ISocksConnectionPtr SocksSessionMng::createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr)
{
  return _server.addConnection(std::addressof(user), addr);
}

void SocksSessionMng::onConnectionDestroyed(ISocksConnectionUser & user, ISocksConnectionPtr conn)
{
  _server.closeConnection(std::addressof(user));
}

void SocksSessionMng::onSessionEnd(uint32_t id)
{
  _sessions.erase(id);
}
