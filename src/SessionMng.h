#ifndef SessionMngH
#define SessionMngH

#include "LoggerAdapter.h"
#include "EventTcpServer.h"
#include "Session.h"
#include "SocksLoginPassAuthorizer.h"
#include <unordered_map>

class SessionMng : public ITcpServerUser, public ISocksSessionUser, private LoggerAdapter
{
public:
  explicit SessionMng(sockaddr_in addr);

  int run();

private:
  EventTcpServer _server;
  uint32_t _currentId;
  SocksLoginPassAuthorizer _authorizer;

  typedef std::unique_ptr<Session> SessionPtr;
  typedef std::unordered_map<uint32_t, SessionPtr> MapSessions;
  MapSessions _sessions;

  //ITcpServerUser
  virtual void onNewConnection(ISocksConnection * newConn) override;

  //ISocksSessionUser
  virtual ISocksConnectionPtr createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr) override;
  virtual void onConnectionDestroyed(ISocksConnectionUser & user, ISocksConnectionPtr conn) override;
  virtual void onSessionEnd(uint32_t id) override;
};

#endif // SessionMngH
