#ifndef SessionH
#define SessionH
//-----------------------------------------------------------------------------
#include "LoggerAdapter.h"
#include "SocksInterfaces.h"
#include "TcpStreamProxy.h"
#include "InConnTracker.h"
//-----------------------------------------------------------------------------
class Session : private LoggerAdapter, private IProxyUser,
                 private IConnTrackerOwner, private ISocksConnectionUser
{
public:
  Session(uint32_t id, ISocksSessionUser & user, ISocksConnectionPtr inConn,
           ISocksAuthorizer & auth);

private:
  const uint32_t _id;
  ISocksSessionUser & _user;
  ISocksAuthorizer & _auth;
  ISocksConnectionPtr _inConn;
  ISocksConnectionPtr _outConn;
  InConnTracker _tracker;
  typedef std::unique_ptr<TcpStreamProxy> TcpStreamProxyPtr;
  TcpStreamProxyPtr _tcpStreamProxy;

  // IProxyUser
  virtual void onProxyDestroy() override;

  // IConnTrackerOwner
  virtual void onStartProxy(SocksCommandCode type, SocksAddress address) override;
  virtual void onRequestPassAuth(const std::string & user, const std::string & password) override;
  virtual void onConnTrackerDestroy() override;

  // ISocksConnectionUser
  virtual void onReceive(const VecByte & buf) override;
  virtual void onConnected(bool connected) override;
  virtual void onConnectionClosed() override;

  void destroySelf(int level, std::string_view reason);
};
//-----------------------------------------------------------------------------
#endif // SessionH
//-----------------------------------------------------------------------------
