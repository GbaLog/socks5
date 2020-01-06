#include <gtest/gtest.h>
#include "Common.h"
#include "SocksDecoder.h"

class SocksDecoderTest : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _decoder = new SocksDecoder({ SocksVersion::Version5 });
  }

  virtual void TearDown() override
  {
    delete _decoder;
  }

  SocksDecoder * _decoder;
};
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, EmptyBuffer)
{
  VecByte vec;

  {
    SocksGreetingMsg tmp;
    EXPECT_FALSE(_decoder->decode(vec, tmp));
  }

  {
    SocksUserPassAuthMsg tmp;
    EXPECT_FALSE(_decoder->decode(vec, tmp));
  }

  {
    SocksCommandMsg tmp;
    EXPECT_FALSE(_decoder->decode(vec, tmp));
  }
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, GreetingMsgSuccess)
{
  VecByte buf
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02  //no auth, gssapi auth, login/pass auth
  };

  SocksGreetingMsg msg;
  ASSERT_TRUE(_decoder->decode(buf, msg));

  EXPECT_EQ(SocksVersion::Version5, msg._version._value);
  ASSERT_EQ(3, msg._authMethods.size());
  EXPECT_EQ(SocksAuthMethod::NoAuth, msg._authMethods[0]._value);
  EXPECT_EQ(SocksAuthMethod::AuthGSSAPI, msg._authMethods[1]._value);
  EXPECT_EQ(SocksAuthMethod::AuthLoginPass, msg._authMethods[2]._value);
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, GreetingMsgWrongVersion)
{
  VecByte buf
  {
    0x04,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02  //no auth, gssapi auth, login/pass auth
  };

  SocksGreetingMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, GreetingMsgMissedMethod)
{
  VecByte buf
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01        //no auth, gssapi auth
  };
  //There is no last method, although three are specified

  SocksGreetingMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, GreetingMsgExcessMethods)
{
  VecByte buf
  {
    0x05,             //version
    0x03,             //number of supported methods
    0x00, 0x01, 0x02, //no auth, gssapi auth, login/pass auth
    0xff, 0xff, 0xff
  };
  //There are excess bytes, although three are specified

  SocksGreetingMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, GreetingMsgIncompleteMethod)
{
  VecByte buf
  {
    0x05,             //version
  };

  SocksGreetingMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, GreetingMsgGarbageMethods)
{
  VecByte buf
  {
    0x05,             //version
    0x03,             //number of supported methods
    0xff, 0xff, 0xff  //garbage
  };

  SocksGreetingMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, UserPassMsgSuccess)
{
  VecByte buf
  {
    0x05,           //version
    0x03,           //username length
    'a', 's', 'd',  //username
    0x03,           //password length
    'q', 'w', 'e'   //password
  };

  SocksUserPassAuthMsg msg;
  ASSERT_TRUE(_decoder->decode(buf, msg));

  EXPECT_EQ(SocksVersion::Version5, msg._version._value);
  ASSERT_EQ(3, msg._user.size());
  EXPECT_EQ("asd", msg._user);
  ASSERT_EQ(3, msg._password.size());
  EXPECT_EQ("qwe", msg._password);
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, UserPassMsgWrongVersion)
{
  VecByte buf
  {
    0x04,           //version
    0x03,           //username length
    'a', 's', 'd',  //username
    0x03,           //password length
    'q', 'w', 'e'   //password
  };

  SocksUserPassAuthMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, UserPassMsgEmptyUserPass)
{
  VecByte buf
  {
    0x05, //version
    0x00, //username length
          //username
    0x00, //password length
          //password
  };

  SocksUserPassAuthMsg msg;
  ASSERT_TRUE(_decoder->decode(buf, msg));

  EXPECT_EQ(SocksVersion::Version5, msg._version._value);
  ASSERT_EQ(0, msg._user.size());
  EXPECT_EQ("", msg._user);
  ASSERT_EQ(0, msg._password.size());
  EXPECT_EQ("", msg._password);
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, UserPassMsgOverlength)
{
  VecByte buf
  {
    0x05,               //version
    0x03,               //username length
    'a', 's', 'd', 'f', //username
    0x03,               //password length
    'q', 'w', 'e'       //password
  };
  //There is excess 'f' symbol in the username

  SocksUserPassAuthMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, UserPassMsgLackingOfPasswordSymbols)
{
  VecByte buf
  {
    0x05,          //version
    0x03,          //username length
    'a', 's', 'd', //username
    0x03,          //password length
    'q', 'w',      //password
  };
  //There is no last symbol in the password

  SocksUserPassAuthMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, UserPassMsgWrongNameLength)
{
  VecByte buf
  {
    0x05,               //version
    0x08,               //username length
    'a', 's', 'd', 'f', //username
    0x03,               //password length
    'q', 'w', 'e'       //password
  };
  //There should be 8 symbols in the username, but there are only 4
  //Such errors are difficult to detect, because there are no any control messages,
  //so we can only rely on the size of the message.
  //Other length tests will be ommited because of this.

  SocksUserPassAuthMsg msg;
  EXPECT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestIPv4TCPPortBindSuccess)
{
  VecByte buf
  {
    0x05,                   //version
    0x02,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4
    0x1a, 0x2b, 0x3c, 0x4d, //IP
    0x11, 0x22              //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_TRUE(_decoder->decode(buf, msg));

  EXPECT_EQ(SocksVersion::Version5, msg._version._value);
  EXPECT_EQ(SocksCommandCode::TCPPortBinding, msg._command._value);
  EXPECT_EQ(SocksAddressType::IPv4Addr, msg._addrType._value);

  SocksIPv4Address & addr = std::get<SocksIPv4Address>(msg._addr);
  EXPECT_EQ(0x1a, addr._value[0]);
  EXPECT_EQ(0x2b, addr._value[1]);
  EXPECT_EQ(0x3c, addr._value[2]);
  EXPECT_EQ(0x4d, addr._value[3]);

  EXPECT_EQ(0x11 << 8 | 0x22, msg._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestWrongVersion)
{
  VecByte buf
  {
    0x03,                   //version
    0x02,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4
    0x1a, 0x2b, 0x3c, 0x4d, //IP
    0x11, 0x22              //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestDomainTCPStreamSuccess)
{
  VecByte buf
  {
    0x05,                                                  //version
    0x01,                                                  //TCP stream connection
    0x00,                                                  //reserved, must be 0x00
    0x03,                                                  //Domain address
    0x0b,                                                  //Domain length
    'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', //Domain
    0x22, 0x33                                             //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_TRUE(_decoder->decode(buf, msg));

  EXPECT_EQ(SocksVersion::Version5, msg._version._value);
  EXPECT_EQ(SocksCommandCode::TCPStream, msg._command._value);
  EXPECT_EQ(SocksAddressType::DomainAddr, msg._addrType._value);

  SocksDomainAddress & addr = std::get<SocksDomainAddress>(msg._addr);
  VecByte domain{ 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm' };
  ASSERT_EQ(domain.size(), addr._value.size());
  EXPECT_EQ(domain, addr._value);

  EXPECT_EQ(0x22 << 8 | 0x33, msg._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestIPv6UdpPortSuccess)
{
  VecByte buf
  {
    0x05,                                                  //version
    0x03,                                                  //UDP port
    0x00,                                                  //reserved, must be 0x00
    0x04,                                                  //IPv6 address
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,        //IPv6
    0x33, 0x44                                             //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_TRUE(_decoder->decode(buf, msg));

  EXPECT_EQ(SocksVersion::Version5, msg._version._value);
  EXPECT_EQ(SocksCommandCode::UDPPortBinding, msg._command._value);
  EXPECT_EQ(SocksAddressType::IPv6Addr, msg._addrType._value);

  SocksIPv6Address & addr = std::get<SocksIPv6Address>(msg._addr);
  EXPECT_EQ(0x00, addr._value[0]);
  EXPECT_EQ(0x01, addr._value[1]);
  EXPECT_EQ(0x02, addr._value[2]);
  EXPECT_EQ(0x03, addr._value[3]);
  EXPECT_EQ(0x04, addr._value[4]);
  EXPECT_EQ(0x05, addr._value[5]);
  EXPECT_EQ(0x06, addr._value[6]);
  EXPECT_EQ(0x07, addr._value[7]);
  EXPECT_EQ(0x08, addr._value[8]);
  EXPECT_EQ(0x09, addr._value[9]);
  EXPECT_EQ(0x0a, addr._value[10]);
  EXPECT_EQ(0x0b, addr._value[11]);
  EXPECT_EQ(0x0c, addr._value[12]);
  EXPECT_EQ(0x0d, addr._value[13]);
  EXPECT_EQ(0x0e, addr._value[14]);
  EXPECT_EQ(0x0f, addr._value[15]);

  EXPECT_EQ(0x33 << 8 | 0x44, msg._port);
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestIPv4MissIPAddressByte)
{
  VecByte buf
  {
    0x05,                   //version
    0x02,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4
    0x1a, 0x2b, 0x3c,       //IP
    0x11, 0x22              //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestDomainMissDomainByte)
{
  VecByte buf
  {
    0x05,                                                  //version
    0x01,                                                  //TCP stream connection
    0x00,                                                  //reserved, must be 0x00
    0x03,                                                  //Domain address
    0x0b,                                                  //Domain length
    'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o',      //Domain
    0x22, 0x33                                             //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestIPv6MissAddrByte)
{
  VecByte buf
  {
    0x05,                                                  //version
    0x03,                                                  //UDP port
    0x00,                                                  //reserved, must be 0x00
    0x04,                                                  //IPv6 address
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,        //IPv6
    0x33, 0x44                                             //port in network byte order
  };
  //IPv6 address requeires 16 bytes of address, there're only 15 bytes

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestMissPort)
{
  VecByte buf
  {
    0x05,                   //version
    0x02,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4 address
    0x1a, 0x2b, 0x3c, 0x4d, //IP
                            //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestWrongCommandCode)
{
  VecByte buf
  {
    0x05,                   //version
    0xff,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0x01,                   //IPv4 address
    0x1a, 0x2b, 0x3c, 0x4d, //IP
    0x11, 0x22              //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestReservedFieldUse)
{
  VecByte buf
  {
    0x05,                   //version
    0x02,                   //TCP port binding
    0x01,                   //reserved, must be 0x00
    0x01,                   //IPv4 address
    0x1a, 0x2b, 0x3c, 0x4d, //IP
    0x11, 0x22              //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
//-----------------------------------------------------------------------------
TEST_F(SocksDecoderTest, ConnRequestWrongAddressType)
{
  VecByte buf
  {
    0x05,                   //version
    0x02,                   //TCP port binding
    0x00,                   //reserved, must be 0x00
    0xff,                   //IPv4 address
    0x1a, 0x2b, 0x3c, 0x4d, //IP
    0x11, 0x22              //port in network byte order
  };

  SocksCommandMsg msg;
  ASSERT_FALSE(_decoder->decode(buf, msg));
}
