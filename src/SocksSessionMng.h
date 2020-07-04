#ifndef SOCKSSESSIONMNG_H
#define SOCKSSESSIONMNG_H

#include "LoggerAdapter.h"
#include "EventTcpServer.h"
#include "SocksSession.h"
#include "SocksLoginPassAuthorizer.h"
#include <unordered_map>

class SocksSessionMng : public ITcpServerUser,
                        public ISocksSessionUser,
                        private LoggerAdapter
{
public:
  explicit SocksSessionMng(sockaddr_in addr);

  int run();

private:
  EventTcpServer _server;
  uint32_t _currentId;
  SocksLoginPassAuthorizer _authorizer;

  typedef std::unique_ptr<SocksSession> SessionPtr;
  struct SessionParams : public ISocksConnectionUser
  {
    uint32_t _id;
    ISocksConnection * _inConn;
    ISocksConnectionPtr _outConn;
    SessionPtr _session;

    virtual void onReceive(const VecByte & buf) override
    { _session->processData(buf); }
    virtual void onConnected(bool connected) override {}
    virtual void onConnectionClosed() override
    { _session->clientDisconnected(); }
  };
  typedef std::unique_ptr<SessionParams> SessionParamsPtr;
  std::unordered_map<uint32_t, SessionParamsPtr> _sessions;

  //ITcpServerUser
  virtual void onNewConnection(ISocksConnection * newConn) override;

  //ISocksSessionUser
  virtual ISocksConnectionPtr createNewConnection(ISocksConnectionUser & user, const SocksAddress & addr) override;
  virtual void onConnectionDestroyed(ISocksConnectionUser & user, ISocksConnectionPtr conn) override;
  virtual void onSessionEnd(uint32_t id) override;
};

#endif // SOCKSSESSIONMNG_H
