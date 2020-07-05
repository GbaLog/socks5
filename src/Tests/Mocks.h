#ifndef MocksH
#define MocksH
//-----------------------------------------------------------------------------
#include <gmock/gmock.h>
#include "SocksInterfaces.h"
#include "StateMachine.h"
#include "InConnTracker.h"
//-----------------------------------------------------------------------------
class SocksConnectionMock : public ISocksConnection
{
public:
  MOCK_METHOD1(setUser, void (ISocksConnectionUser * user));
  MOCK_METHOD0(connect, bool ());
  MOCK_METHOD1(send, bool (const VecByte & buf));
  MOCK_METHOD0(closeConnection, void ());
  MOCK_CONST_METHOD0(isConnected, bool ());
  MOCK_CONST_METHOD0(getLocalAddress, std::optional<SocksAddress> ());
};
//-----------------------------------------------------------------------------
class SocksSessionUserMock : public ISocksSessionUser
{
public:
  MOCK_METHOD2(createNewConnection, SocksConnectionPtr (ISocksConnectionUser & user, const SocksAddress & addr));
  MOCK_METHOD2(onConnectionDestroyed, void (ISocksConnectionUser & user, SocksConnectionPtr conn));
  MOCK_METHOD1(onSessionEnd, void (uint32_t id));
};
//-----------------------------------------------------------------------------
class SocksAuthorizerMock : public ISocksAuthorizer
{
public:
  MOCK_CONST_METHOD1(isMethodSupported, bool (const SocksAuthMethod & method));
  MOCK_CONST_METHOD2(authUserPassword, bool (const std::string & user, const std::string & pass));
};
//-----------------------------------------------------------------------------
class StateMachineOwnerMock : public IStateMachineOwner
{
public:
  MOCK_METHOD1(sendGreetingResponse, void (SocksAuthMethod method));
  MOCK_METHOD2(requestPassAuth,      void (const std::string & user, const std::string & password));
  MOCK_METHOD1(sendPassAuthResponse, void (Byte status));
  MOCK_METHOD2(startProxy,           void (SocksCommandCode type, const SocksAddress & address));
  MOCK_METHOD2(sendCommandResponse,  void (Byte status, const SocksAddress & localAddress));
  MOCK_METHOD1(onProtocolError,      void (const std::string & reason));
};
//-----------------------------------------------------------------------------
class IConnTrackerOwnerMock : public IConnTrackerOwner
{
public:
  MOCK_METHOD2(onStartProxy, void (SocksCommandCode type, SocksAddress address));
  MOCK_METHOD2(onRequestPassAuth, void (const std::string & user, const std::string & password));
  MOCK_METHOD0(onConnTrackerDestroy, void ());
};

#endif // MocksH
