#include "SessionMng2.h"

SessionMng2::SessionMng2(sockaddr_in addr) :
  LoggerAdapter("SocksSessMng"),
  _server(*this, addr),
  _currentId(0),
  _authorizer("logins.txt")
{}

int SessionMng2::run()
{
  return _server.run();
}

void SessionMng2::onNewConnection(ISocksConnection * newConn)
{
  log(DBG, "Create new connection");
  auto id = _currentId++;
  ISocksConnectionPtr connPtr{newConn};
  auto newSess = std::make_unique<Session2>(id, *this, connPtr, _authorizer);
  _sessions[id] = std::move(newSess);
}

ISocksConnectionPtr SessionMng2::createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr)
{
  return _server.addConnection(std::addressof(user), addr);
}

void SessionMng2::onConnectionDestroyed(ISocksConnectionUser & user, ISocksConnectionPtr conn)
{
  _server.closeConnection(std::addressof(user));
}

void SessionMng2::onSessionEnd(uint32_t id)
{
  if (_sessions.erase(id) > 0)
  {
    log(DBG, "Session with id: {} erased", id);
  }
}
