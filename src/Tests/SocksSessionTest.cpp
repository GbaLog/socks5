#include "Mocks.h"
#include "SocksSession.h"

using ::testing::Return;
using ::testing::InSequence;

class SocksSessionTest : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _incomingConn = new SocksConnectionMock;
    _sessUser = new SocksSessionUserMock;
    _auth = new SocksAuthorizerMock;
    _session = new SocksSession(*_sessUser, *_incomingConn, *_auth);
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

TEST_F(SocksSessionTest, GreetingOnly)
{
  VecByte greeting
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02  //no auth, gssapi auth, login/pass auth
  };

  VecByte expectedAnswer
  {
    0x05,
    0x01
  };

  EXPECT_CALL(*_incomingConn, send(expectedAnswer)).WillOnce(Return(true));

  _session->processData(greeting);
}

TEST_F(SocksSessionTest, AuthenticationSuccess)
{
  VecByte greeting
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02  //no auth, gssapi auth, login/pass auth
  };

  VecByte auth
  {
    0x05,           //version
    0x03,           //username length
    'a', 's', 'd',  //username
    0x03,           //password length
    'q', 'w', 'e'   //password
  };

  VecByte greetingAnswer
  {
    0x05,
    0x01
  };

  VecByte authAnswer
  {
    0x05,
    0x00
  };

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
  }

  _session->processData(greeting);
  _session->processData(auth);
}

TEST_F(SocksSessionTest, AuthenticationFailed)
{
  VecByte greeting
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02  //no auth, gssapi auth, login/pass auth
  };

  VecByte auth
  {
    0x05,           //version
    0x03,           //username length
    'a', 's', 'd',  //username
    0x03,           //password length
    'q', 'w', 'e'   //password
  };

  VecByte greetingAnswer
  {
    0x05,
    0x01
  };

  VecByte authAnswer
  {
    0x05,
    0x01
  };

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(false));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, closeConnection()).Times(1);
  }

  _session->processData(greeting);
  _session->processData(auth);
}

TEST_F(SocksSessionTest, ConnectCommandSuccess)
{
  VecByte greeting
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02  //no auth, gssapi auth, login/pass auth
  };

  VecByte auth
  {
    0x05,           //version
    0x03,           //username length
    'a', 's', 'd',  //username
    0x03,           //password length
    'q', 'w', 'e'   //password
  };

  VecByte cmd
  {
    0x05,                   //version
    0x02,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4
    0x1a, 0x2b, 0x3c, 0x4d, //IP
    0x11, 0x22              //port in network byte order
  };

  VecByte greetingAnswer
  {
    0x05,
    0x01
  };

  VecByte authAnswer
  {
    0x05,
    0x00
  };

  {
    InSequence seq;
    EXPECT_CALL(*_incomingConn, send(greetingAnswer)).WillOnce(Return(true));
    EXPECT_CALL(*_auth, authUserPassword("asd", "qwe")).WillOnce(Return(true));
    EXPECT_CALL(*_incomingConn, send(authAnswer)).WillOnce(Return(true));
  }

  _session->processData(greeting);
  _session->processData(auth);
}
