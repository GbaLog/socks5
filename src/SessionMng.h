#ifndef SessionMngH
#define SessionMngH
//-----------------------------------------------------------------------------
#include "LoggerAdapter.h"
#include "EventTcpServer.h"
#include "Session.h"
#include "SocksLoginPassAuthorizer.h"
#include "EventBaseObject.h"
#include "EventSignalListener.h"
#include <unordered_map>
//-----------------------------------------------------------------------------
class SessionMng : private LoggerAdapter,
                   private ITcpServerUser,
                   public ISocksSessionUser,
                   private ISignalListenerUser
{
public:
  explicit SessionMng(const sockaddr * saddr, int salen, const std::string & authFilename);

  int run();

private:
  EventBaseObject _base;
  EventTcpServer _server;
  EventSignalListener _sigListener;
  uint32_t _currentId;
  SocksLoginPassAuthorizer _authorizer;

  typedef std::unique_ptr<Session> SessionPtr;
  typedef std::unordered_map<uint32_t, SessionPtr> MapSessions;
  MapSessions _sessions;

  //ITcpServerUser
  virtual void onNewConnection(SocksConnectionPtr newConn) override;

  //ISocksSessionUser
  virtual SocksConnectionPtr createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr) override;
  virtual void onConnectionDestroyed(ISocksConnectionUser & user, SocksConnectionPtr conn) override;
  virtual void onSessionEnd(uint32_t id) override;

  //ISignalListenerUser
  virtual void onSignalOccured(int signum) override;
};
//-----------------------------------------------------------------------------
#endif // SessionMngH
//-----------------------------------------------------------------------------
