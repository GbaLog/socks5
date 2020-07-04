#include "SessionMng.h"
//-----------------------------------------------------------------------------
SessionMng::SessionMng(sockaddr_in addr) :
  LoggerAdapter("SocksSessMng"),
  _server(*this, addr),
  _currentId(0),
  _authorizer("logins.txt")
{}
//-----------------------------------------------------------------------------
int SessionMng::run()
{
  return _server.run();
}
//-----------------------------------------------------------------------------
void SessionMng::onNewConnection(ISocksConnection * newConn)
{
  log(DBG, "Create new connection");
  auto id = _currentId++;
  ISocksConnectionPtr connPtr{newConn};
  auto newSess = std::make_unique<Session>(id, *this, connPtr, _authorizer);
  _sessions[id] = std::move(newSess);
}
//-----------------------------------------------------------------------------
ISocksConnectionPtr SessionMng::createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr)
{
  return _server.addConnection(std::addressof(user), addr);
}
//-----------------------------------------------------------------------------
void SessionMng::onConnectionDestroyed(ISocksConnectionUser & user, ISocksConnectionPtr conn)
{
  _server.closeConnection(std::addressof(user));
}
//-----------------------------------------------------------------------------
void SessionMng::onSessionEnd(uint32_t id)
{
  if (_sessions.erase(id) > 0)
  {
    log(DBG, "Session with id: {} erased", id);
  }
}
//-----------------------------------------------------------------------------
