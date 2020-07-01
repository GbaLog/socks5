#include "Mocks.h"
#include "StateMachine.h"
#include "InetUtils.h"
//-----------------------------------------------------------------------------
using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;
using ::testing::Action;
//-----------------------------------------------------------------------------
class StateMachineTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    _owner = new StateMachineOwnerMock;
    _machine = new StateMachine(0, *_owner);
  }

  void TearDown() override
  {
    delete _machine;
    delete _owner;
  }

protected:
  StateMachineOwnerMock * _owner;
  StateMachine * _machine;
};
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
VecAuthMethod fullAuthMethods
{
  SocksAuthMethod{ SocksAuthMethod::NoAuth },
  SocksAuthMethod{ SocksAuthMethod::AuthLoginPass },
  SocksAuthMethod{ SocksAuthMethod::AuthGSSAPI }
};
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
TEST_F(StateMachineTest, GreetingOnly)
{
  SocksGreetingMsg msg;
  msg._version._value = SocksVersion::Version5;
  msg._authMethods = fullAuthMethods;

  auto expectedMethod = SocksAuthMethod{SocksAuthMethod::AuthLoginPass};
  EXPECT_CALL(*_owner, sendGreetingResponse(expectedMethod)).Times(1);

  _machine->processGreetingMsg(msg);
}
//-----------------------------------------------------------------------------
TEST_F(StateMachineTest, AuthenticationRequest)
{
  SocksGreetingMsg greetingMsg;
  greetingMsg._version._value = SocksVersion::Version5;
  greetingMsg._authMethods = fullAuthMethods;

  {
    InSequence seq;
    auto expectedMethod = SocksAuthMethod{SocksAuthMethod::AuthLoginPass};
    EXPECT_CALL(*_owner, sendGreetingResponse(expectedMethod)).Times(1);
    EXPECT_CALL(*_owner, requestPassAuth("hello", "there")).Times(1);
  }

  _machine->processGreetingMsg(greetingMsg);

  SocksUserPassAuthMsg authMsg;
  authMsg._user = "hello";
  authMsg._password = "there";

  _machine->processPassAuthMsg(authMsg);
}
//-----------------------------------------------------------------------------
TEST_F(StateMachineTest, AuthenticationSuccess)
{
  SocksGreetingMsg greetingMsg;
  greetingMsg._version._value = SocksVersion::Version5;
  greetingMsg._authMethods = fullAuthMethods;

  {
    InSequence seq;
    auto expectedMethod = SocksAuthMethod{SocksAuthMethod::AuthLoginPass};
    EXPECT_CALL(*_owner, sendGreetingResponse(expectedMethod)).Times(1);
    EXPECT_CALL(*_owner, requestPassAuth("hello", "there")).Times(1);
    EXPECT_CALL(*_owner, sendPassAuthResponse(0x00)).Times(1);
  }

  _machine->processGreetingMsg(greetingMsg);

  SocksUserPassAuthMsg authMsg;
  authMsg._user = "hello";
  authMsg._password = "there";

  _machine->processPassAuthMsg(authMsg);
  _machine->processPassAuthResult(true);
}
//-----------------------------------------------------------------------------
TEST_F(StateMachineTest, CommandRequest)
{
  SocksGreetingMsg greetingMsg;
  greetingMsg._version._value = SocksVersion::Version5;
  greetingMsg._authMethods = fullAuthMethods;

  {
    InSequence seq;
    auto expectedMethod = SocksAuthMethod{SocksAuthMethod::AuthLoginPass};
    auto expectedCmdCode = SocksCommandCode{ SocksCommandCode::TCPStream };
    SocksAddress expectedAddress;
    expectedAddress._type._value = SocksAddressType::IPv4Addr;
    expectedAddress._addr = SocksIPv4Address{ 0xde, 0xad, 0xbe, 0xef };
    expectedAddress._port = 35555;
    EXPECT_CALL(*_owner, sendGreetingResponse(expectedMethod)).Times(1);
    EXPECT_CALL(*_owner, requestPassAuth("hello", "there")).Times(1);
    EXPECT_CALL(*_owner, startProxy(expectedCmdCode, expectedAddress)).Times(1);
  }

  _machine->processGreetingMsg(greetingMsg);

  SocksUserPassAuthMsg authMsg;
  authMsg._user = "hello";
  authMsg._password = "there";

  _machine->processPassAuthMsg(authMsg);
  _machine->processPassAuthResult(true);

  SocksCommandMsg cmdMsg;
  cmdMsg._version._value = SocksVersion::Version5;
  cmdMsg._addrType._value = SocksAddressType::IPv4Addr;
  cmdMsg._addr = SocksIPv4Address{ 0xde, 0xad, 0xbe, 0xef };
  cmdMsg._port = 35555;
  cmdMsg._command._value = SocksCommandCode::TCPStream;

  _machine->processCommandMsg(cmdMsg);
}
//-----------------------------------------------------------------------------
TEST_F(StateMachineTest, CommandRequestSuccess)
{
  SocksGreetingMsg greetingMsg;
  greetingMsg._version._value = SocksVersion::Version5;
  greetingMsg._authMethods = fullAuthMethods;

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0xde, 0xad, 0xf0, 0x0d };
  localAddr._port = 56333;

  {
    InSequence seq;
    auto expectedMethod = SocksAuthMethod{SocksAuthMethod::AuthLoginPass};
    auto expectedCmdCode = SocksCommandCode{ SocksCommandCode::TCPStream };
    SocksAddress expectedAddress;
    expectedAddress._type._value = SocksAddressType::IPv4Addr;
    expectedAddress._addr = SocksIPv4Address{ 0xde, 0xad, 0xbe, 0xef };
    expectedAddress._port = 35555;
    EXPECT_CALL(*_owner, sendGreetingResponse(expectedMethod)).Times(1);
    EXPECT_CALL(*_owner, requestPassAuth("hello", "there")).Times(1);
    EXPECT_CALL(*_owner, startProxy(expectedCmdCode, expectedAddress)).Times(1);
    EXPECT_CALL(*_owner, sendCommandResponse(SocksCommandMsgResp::RequestGranted, localAddr)).Times(1);
  }

  _machine->processGreetingMsg(greetingMsg);

  SocksUserPassAuthMsg authMsg;
  authMsg._user = "hello";
  authMsg._password = "there";

  _machine->processPassAuthMsg(authMsg);
  _machine->processPassAuthResult(true);

  SocksCommandMsg cmdMsg;
  cmdMsg._version._value = SocksVersion::Version5;
  cmdMsg._addrType._value = SocksAddressType::IPv4Addr;
  cmdMsg._addr = SocksIPv4Address{ 0xde, 0xad, 0xbe, 0xef };
  cmdMsg._port = 35555;
  cmdMsg._command._value = SocksCommandCode::TCPStream;

  _machine->processCommandMsg(cmdMsg);
  _machine->processStartProxyResult(SocksCommandMsgResp::RequestGranted, localAddr);
}
//-----------------------------------------------------------------------------
TEST_F(StateMachineTest, CommandWithoutAuth)
{
  SocksGreetingMsg greetingMsg;
  greetingMsg._version._value = SocksVersion::Version5;
  greetingMsg._authMethods = fullAuthMethods;

  {
    InSequence seq;
    auto expectedMethod = SocksAuthMethod{SocksAuthMethod::AuthLoginPass};
    EXPECT_CALL(*_owner, sendGreetingResponse(expectedMethod)).Times(1);
    EXPECT_CALL(*_owner, onProtocolError(_)).Times(1);
  }

  _machine->processGreetingMsg(greetingMsg);

  SocksCommandMsg cmdMsg;
  cmdMsg._version._value = SocksVersion::Version5;
  cmdMsg._addrType._value = SocksAddressType::IPv4Addr;
  cmdMsg._addr = SocksIPv4Address{ 0xde, 0xad, 0xbe, 0xef };
  cmdMsg._port = 35555;
  cmdMsg._command._value = SocksCommandCode::TCPStream;

  _machine->processCommandMsg(cmdMsg);
}
