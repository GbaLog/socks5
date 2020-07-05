#include "SessionMng.h"
//-----------------------------------------------------------------------------
SessionMng::SessionMng(const sockaddr * saddr, int salen, const std::string & authFilename) :
  LoggerAdapter("SocksSessMng"),
  _server(*this, _base, saddr, salen),
  _sigListener(*this, _base),
  _currentId(0),
  _authorizer(authFilename)
{}
//-----------------------------------------------------------------------------
int SessionMng::run()
{
  log(INF, "Run session manager, add SIGINT({}) handle", SIGINT);
  _sigListener.add(SIGINT);
  return _server.run();
}
//-----------------------------------------------------------------------------
void SessionMng::onNewConnection(SocksConnectionPtr newConn)
{
  log(DBG, "Create new connection");
  auto id = _currentId++;
  auto newSess = std::make_unique<Session>(id, *this, newConn, _authorizer);
  _sessions[id] = std::move(newSess);
}
//-----------------------------------------------------------------------------
SocksConnectionPtr SessionMng::createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr)
{
  return _server.addConnection(std::addressof(user), addr);
}
//-----------------------------------------------------------------------------
void SessionMng::onConnectionDestroyed(ISocksConnectionUser & user, SocksConnectionPtr conn)
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
void SessionMng::onSignalOccured(int signum)
{
  log(INF, "Signal occured: {}", signum);
  if (signum == SIGINT)
    _server.stop();
}
//-----------------------------------------------------------------------------
