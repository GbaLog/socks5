#include "Mocks.h"
#include "SocksSession.h"
//-----------------------------------------------------------------------------
using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;
using ::testing::Action;
//-----------------------------------------------------------------------------
class SocksSessionTest : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _incomingConn = new SocksConnectionMock;
    _sessUser = new SocksSessionUserMock;
    _auth = new SocksAuthorizerMock;
    _session = new SocksSession(0, *_sessUser, *_incomingConn, *_auth);
  }

  virtual void TearDown() override
  {
    delete _session;
    delete _auth;
    delete _sessUser;
    delete _incomingConn;
  }

  SocksConnectionMock * _incomingConn;
  SocksSessionUserMock * _sessUser;
  SocksAuthorizerMock * _auth;
  SocksSession * _session;
};
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
ACTION_P2(createConnectionGetter, addr, retConn)
{
  *addr = arg1;
  return retConn.lock();
}
//-----------------------------------------------------------------------------
const VecByte usualGreeting
{
  0x05,             //version
  0x03,             //number of supported methods
  0x00, 0x02, 0x01  //no auth, gssapi auth, login/pass auth
};
//-----------------------------------------------------------------------------
const VecByte usualGreetingAnswer
{
  0x05,             //version
  0x01              //login/pass auth
};
//-----------------------------------------------------------------------------
const VecByte usualAuth
{
  0x05,           //version
  0x03,           //username length
  'a', 's', 'd',  //username
  0x03,           //password length
  'q', 'w', 'e'   //password
};
//-----------------------------------------------------------------------------
VecByte usualAuthAnswerOk
{
  0x05,           //version
  0x00            //ok
};
//-----------------------------------------------------------------------------
VecByte usualAuthAnswerFail
{
  0x05,           //version
  0x01            //fail
};
//-----------------------------------------------------------------------------
const VecByte usualCmdTcpPortIPv4
{
  0x05,                   //version
  0x02,                   //TCP port binding
  0x00,                   //reserved, must be 0x00
  0x01,                   //IPv4
  0x1a, 0x2b, 0x3c, 0x4d, //IP
  0x11, 0x22              //port in network byte order
};
//-----------------------------------------------------------------------------
const VecByte usualCmdAnswerOk
{
  0x05,                   //version
  0x00,                   //status OK
  0x00,                   //reserved, must be 0x00
  0x01,                   //IPv4
  0x7f, 0x00, 0x00, 0x01, //IPv4 Localhost
  0x27, 0x10              //Port 10000
};
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, GreetingOnly)
{
  VecByte greeting = usualGreeting;
  VecByte greetingAnswer = usualGreetingAnswer;

  EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));

  _session->processData(greeting);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, AuthenticationSuccess)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
  }

  _session->processData(greeting);
  _session->processData(auth);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, AuthenticationFailed)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerFail;

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(false));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  _session->processData(greeting);
  _session->processData(auth);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, ConnectCommandSuccess)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte cmd = usualCmdTcpPortIPv4;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmdAnswer = usualCmdAnswerOk;

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x01 }; //127.0.0.1
  localAddr._port = 0x2710; //10000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));
    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));
  }

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);

  ASSERT_EQ(SocksAddressType::IPv4Addr, addr._type._value);
  SocksIPv4Address ipv4 = std::get<SocksIPv4Address>(addr._addr);
  EXPECT_EQ(0x1a, ipv4._value[0]);
  EXPECT_EQ(0x2b, ipv4._value[1]);
  EXPECT_EQ(0x3c, ipv4._value[2]);
  EXPECT_EQ(0x4d, ipv4._value[3]);
  EXPECT_EQ(0x11 << 8 | 0x22, addr._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, ConnectCommandSuccessWithFurtherData)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte cmd = usualCmdTcpPortIPv4;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmdAnswer
  {
    0x05,
    0x00,
    0x00,
    0x01,
    0x7f, 0x00, 0x00, 0x02,
    0x4e, 0x20
  };

  std::string dataStr =
      "GET / HTTP/2.0\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";

  std::string dataAnswerStr =
      "200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "{}\r\n";

  VecByte data{ dataStr.begin(), dataStr.end() };
  VecByte dataAnswer{ dataAnswerStr.begin(), dataAnswerStr.end() };

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x02 }; //127.0.0.2
  localAddr._port = 0x4e20; //20000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));
    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));

    //First proxying packet has been received
    EXPECT_CALL(*conn, send(data)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(dataAnswer)).WillOnce(Return(true));
  }

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
  _session->processData(data);
  //Imitate answer
  static_cast<ISocksConnectionUser *>(_session)->onReceive(dataAnswer);

  ASSERT_EQ(SocksAddressType::IPv4Addr, addr._type._value);
  SocksIPv4Address ipv4 = std::get<SocksIPv4Address>(addr._addr);
  EXPECT_EQ(0x1a, ipv4._value[0]);
  EXPECT_EQ(0x2b, ipv4._value[1]);
  EXPECT_EQ(0x3c, ipv4._value[2]);
  EXPECT_EQ(0x4d, ipv4._value[3]);
  EXPECT_EQ(0x11 << 8 | 0x22, addr._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, NoSupportedAuthMethods)
{
  VecByte greeting
  {
    0x05,             //version
    0x01,             //number of supported methods
    0x02              //gssapi auth
  };

  VecByte greetingAnswer
  {
    0x05,             //version
    0xff              //no acceptable methods
  };

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  _session->processData(greeting);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, UserDisconnectWhileGreeting)
{
  VecByte greeting = usualGreeting;
  VecByte greetingAnswer = usualGreetingAnswer;

  {
    InSequence seq;
    //answer will be sended anyway
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  _session->processData(greeting);
  _session->clientDisconnected();
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, UserDisconnectWhileAuth)
{
  VecByte greeting = usualGreeting;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte auth = usualAuth;
  VecByte authAnswer = usualAuthAnswerOk;

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  _session->processData(greeting);
  _session->processData(auth);
  _session->clientDisconnected();
}
//-----------------------------------------------------------------------------
//This test has to be uncommented when async connect is made
/*
TEST_F(SocksSessionTest, UserDisconnectWhileConnect)
{
  VecByte greeting = usualGreeting;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte auth = usualAuth;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmd = usualCmdTcpPortIPv4;

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x02 }; //127.0.0.2
  localAddr._port = 0x4e20; //20000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));

    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));

    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));

    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
  EXPECT_CALL(*conn, closeConnection()).Times(1);

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
  _session->clientDisconnected();
}
*/
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, UserDisconnectAfterSuccessfulDataSending)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte cmd = usualCmdTcpPortIPv4;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmdAnswer
  {
    0x05,
    0x00,
    0x00,
    0x01,
    0x7f, 0x00, 0x00, 0x02,
    0x4e, 0x20
  };

  std::string dataStr =
      "GET / HTTP/2.0\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";

  VecByte data{ dataStr.begin(), dataStr.end() };

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x02 }; //127.0.0.2
  localAddr._port = 0x4e20; //20000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));
    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));

    //First proxying packet has been received
    EXPECT_CALL(*conn, send(data)).WillOnce(Return(true));
    //Disconnect happens
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  EXPECT_CALL(*conn, closeConnection()).Times(1);
  EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
  _session->processData(data);
  _session->clientDisconnected();

  ASSERT_EQ(SocksAddressType::IPv4Addr, addr._type._value);
  SocksIPv4Address ipv4 = std::get<SocksIPv4Address>(addr._addr);
  EXPECT_EQ(0x1a, ipv4._value[0]);
  EXPECT_EQ(0x2b, ipv4._value[1]);
  EXPECT_EQ(0x3c, ipv4._value[2]);
  EXPECT_EQ(0x4d, ipv4._value[3]);
  EXPECT_EQ(0x11 << 8 | 0x22, addr._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, DestUserDisconnectBeforeDataReceiving)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte cmd = usualCmdTcpPortIPv4;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmdAnswer
  {
    0x05,
    0x00,
    0x00,
    0x01,
    0x7f, 0x00, 0x00, 0x02,
    0x4e, 0x20
  };

  std::string dataStr =
      "GET / HTTP/2.0\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";

  VecByte data{ dataStr.begin(), dataStr.end() };

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x02 }; //127.0.0.2
  localAddr._port = 0x4e20; //20000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));
    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));

    //Destination disconnect
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
  EXPECT_CALL(*conn, closeConnection()).Times(1);

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
  static_cast<ISocksConnectionUser *>(_session)->onConnectionClosed();

  ASSERT_EQ(SocksAddressType::IPv4Addr, addr._type._value);
  SocksIPv4Address ipv4 = std::get<SocksIPv4Address>(addr._addr);
  EXPECT_EQ(0x1a, ipv4._value[0]);
  EXPECT_EQ(0x2b, ipv4._value[1]);
  EXPECT_EQ(0x3c, ipv4._value[2]);
  EXPECT_EQ(0x4d, ipv4._value[3]);
  EXPECT_EQ(0x11 << 8 | 0x22, addr._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, DestUserDisconnectBeforeAnswer)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte cmd = usualCmdTcpPortIPv4;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmdAnswer
  {
    0x05,
    0x00,
    0x00,
    0x01,
    0x7f, 0x00, 0x00, 0x02,
    0x4e, 0x20
  };

  std::string dataStr =
      "GET / HTTP/2.0\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";

  VecByte data{ dataStr.begin(), dataStr.end() };

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x02 }; //127.0.0.2
  localAddr._port = 0x4e20; //20000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));
    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));

    //First proxying packet has been received
    EXPECT_CALL(*conn, send(data)).WillOnce(Return(true));
    //Destination disconnect
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
  EXPECT_CALL(*conn, closeConnection()).Times(1);

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
  _session->processData(data);
  static_cast<ISocksConnectionUser *>(_session)->onConnectionClosed();

  ASSERT_EQ(SocksAddressType::IPv4Addr, addr._type._value);
  SocksIPv4Address ipv4 = std::get<SocksIPv4Address>(addr._addr);
  EXPECT_EQ(0x1a, ipv4._value[0]);
  EXPECT_EQ(0x2b, ipv4._value[1]);
  EXPECT_EQ(0x3c, ipv4._value[2]);
  EXPECT_EQ(0x4d, ipv4._value[3]);
  EXPECT_EQ(0x11 << 8 | 0x22, addr._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, DestUserDisconnectRightAfterAnswer)
{
  VecByte greeting = usualGreeting;
  VecByte auth = usualAuth;
  VecByte cmd = usualCmdTcpPortIPv4;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte authAnswer = usualAuthAnswerOk;
  VecByte cmdAnswer
  {
    0x05,
    0x00,
    0x00,
    0x01,
    0x7f, 0x00, 0x00, 0x02,
    0x4e, 0x20
  };

  std::string dataStr =
      "GET / HTTP/2.0\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";

  std::string dataAnswerStr =
      "200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "{}\r\n";

  VecByte data{ dataStr.begin(), dataStr.end() };
  VecByte dataAnswer{ dataAnswerStr.begin(), dataAnswerStr.end() };

  SocksAddress localAddr;
  localAddr._type._value = SocksAddressType::IPv4Addr;
  localAddr._addr = SocksIPv4Address{ 0x7f, 0x00, 0x00, 0x02 }; //127.0.0.2
  localAddr._port = 0x4e20; //20000

  SocksAddress addr;
  auto conn = std::make_shared<SocksConnectionMock>();

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_sessUser, createNewConnection(_, _))
        .WillOnce(createConnectionGetter(&addr, std::weak_ptr{conn}));
    EXPECT_CALL(*conn, connect()).WillOnce(Return(true));
    EXPECT_CALL(*conn, getLocalAddress()).WillOnce(Return(localAddr));
    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));

    //First proxying packet has been received
    EXPECT_CALL(*conn, send(data)).WillOnce(Return(true));
    //Destination answer received
    EXPECT_CALL(*_incomingConn, send(dataAnswer)).WillOnce(Return(true));
    //Destination disconnect
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
  EXPECT_CALL(*conn, closeConnection()).Times(1);

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
  _session->processData(data);
  static_cast<ISocksConnectionUser *>(_session)->onReceive(dataAnswer);
  static_cast<ISocksConnectionUser *>(_session)->onConnectionClosed();

  ASSERT_EQ(SocksAddressType::IPv4Addr, addr._type._value);
  SocksIPv4Address ipv4 = std::get<SocksIPv4Address>(addr._addr);
  EXPECT_EQ(0x1a, ipv4._value[0]);
  EXPECT_EQ(0x2b, ipv4._value[1]);
  EXPECT_EQ(0x3c, ipv4._value[2]);
  EXPECT_EQ(0x4d, ipv4._value[3]);
  EXPECT_EQ(0x11 << 8 | 0x22, addr._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, GarbageInsteadOfGreeting)
{
  VecByte greeting
  {
    0xff, 0x3d, 0x4d, 0x5a, 0x33
  };

  EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
  EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);

  _session->processData(greeting);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, GarbageInsteadOfAuthentication)
{
  VecByte greeting = usualGreeting;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte auth
  {
    0xff, 0x3d, 0x4d, 0x5a, 0x33, 0x43, 0xfa, 0xa1
  };


  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  _session->processData(greeting);
  _session->processData(auth);
}
//-----------------------------------------------------------------------------
TEST_F(SocksSessionTest, GarbageInsteadOfCommand)
{
  VecByte greeting = usualGreeting;
  VecByte greetingAnswer = usualGreetingAnswer;
  VecByte auth = usualAuth;
  VecByte authAnswer = usualAuthAnswerOk;

  VecByte cmd
  {
    0xff, 0x3d, 0x4d, 0x5a, 0x33, 0x43, 0xfa, 0xa1,
    0x35, 0xc2, 0xd1, 0x76, 0x42, 0xaa, 0x16, 0x08
  };

  VecByte cmdAnswer
  {
    0x05,                   //version
    0x07,                   //status ProtocolError
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4
    0x00, 0x00, 0x00, 0x00, //IPv4 0
    0x00, 0x00              //Port 0
  };

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));

    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));

    EXPECT_CALL(*_incomingConn, send(cmdAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
    EXPECT_CALL(*_sessUser, onConnectionDestroyed(_, _)).Times(1);
  }

  _session->processData(greeting);
  _session->processData(auth);
  _session->processData(cmd);
}
