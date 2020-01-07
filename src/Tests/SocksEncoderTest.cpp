#include <gtest/gtest.h>
#include "Common.h"
#include "SocksEncoder.h"
//-----------------------------------------------------------------------------
class SocksEncoderTest : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _encoder = new SocksEncoder({ SocksVersion::Version5 });
  }

  virtual void TearDown() override
  {
    delete _encoder;
  }

  SocksEncoder * _encoder;
};
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, WrongVersion)
{
  VecByte buf;

  {
    SocksGreetingMsgResp msg;
    msg._version._value = 0xf1;
    EXPECT_FALSE(_encoder->encode(msg, buf));
  }

  {
    SocksUserPassAuthMsgResp msg;
    msg._version._value = 0xf2;
    EXPECT_FALSE(_encoder->encode(msg, buf));
  }

  {
    SocksCommandMsgResp msg;
    msg._version._value = 0xf3;
    EXPECT_FALSE(_encoder->encode(msg, buf));
  }
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, GreetingSuccess)
{
  VecByte buf;
  SocksGreetingMsgResp msg;

  msg._version._value = 0x05;
  msg._authMethod._value = 0x00;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(2, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(SocksAuthMethod::NoAuth, buf[1]);
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, GreetingUnknownMethod)
{
  VecByte buf;
  SocksGreetingMsgResp msg;

  msg._version._value = 0x05;
  msg._authMethod._value = 0x33;

  ASSERT_FALSE(_encoder->encode(msg, buf));
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, UserPassAuthSuccess)
{
  VecByte buf;
  SocksUserPassAuthMsgResp msg;

  msg._version._value = 0x05;
  msg._status = 0x00;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(2, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(0x00, buf[1]);
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, UserPassAuthFailStatus)
{
  VecByte buf;
  SocksUserPassAuthMsgResp msg;

  msg._version._value = 0x05;
  msg._status = 0x01;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(2, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(0x01, buf[1]);
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, CommandIPv4Success)
{
  VecByte buf;
  SocksCommandMsgResp msg;

  /*
    0x05,                   //version
    0x00,                   //status
    0x00,                   //reserved, must be 0x00
    0x01,                   //address type, IPv4
    0x1a, 0x2b, 0x3c, 0x4d, //IPv4
    0xaa, 0xbb              //port
  */

  msg._version._value = 0x05;
  msg._status = 0x00;
  msg._addrType = { SocksAddressType::IPv4Addr };
  msg._addr = SocksIPv4Address{ { 0x1a, 0x2b, 0x3c, 0x4d } };
  msg._port = 0xaabb;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(10, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(0x00, buf[1]); //status
  EXPECT_EQ(0x00, buf[2]); //reserved
  EXPECT_EQ(SocksAddressType::IPv4Addr, buf[3]);
  EXPECT_EQ(0x1a, buf[4]);
  EXPECT_EQ(0x2b, buf[5]);
  EXPECT_EQ(0x3c, buf[6]);
  EXPECT_EQ(0x4d, buf[7]);
  EXPECT_EQ(0xaa, buf[8]);
  EXPECT_EQ(0xbb, buf[9]);
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, CommandDomainSuccess)
{
  VecByte buf;
  SocksCommandMsgResp msg;

  /*
    0x05,                                                   //version
    0x00,                                                   //status
    0x00,                                                   //reserved, must be 0x00
    0x03,                                                   //address type, domain
    0x0b,                                                   //domain length
    'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm',  //domain
    0xa1, 0xb2                                              //port
  */

  msg._version._value = 0x05;
  msg._status = 0x00;
  msg._addrType = { SocksAddressType::DomainAddr };
  msg._addr = SocksDomainAddress{ { 'e', 'x', 'a', 'm', 'p','l', 'e', '.', 'c', 'o', 'm' } };
  msg._port = 0xa1b2;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(18, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(0x00, buf[1]); //status
  EXPECT_EQ(0x00, buf[2]); //reserved
  EXPECT_EQ(SocksAddressType::DomainAddr, buf[3]);
  EXPECT_EQ(0x0b, buf[4]);
  EXPECT_EQ('e', buf[5]);
  EXPECT_EQ('x', buf[6]);
  EXPECT_EQ('a', buf[7]);
  EXPECT_EQ('m', buf[8]);
  EXPECT_EQ('p', buf[9]);
  EXPECT_EQ('l', buf[10]);
  EXPECT_EQ('e', buf[11]);
  EXPECT_EQ('.', buf[12]);
  EXPECT_EQ('c', buf[13]);
  EXPECT_EQ('o', buf[14]);
  EXPECT_EQ('m', buf[15]);
  EXPECT_EQ(0xa1, buf[16]);
  EXPECT_EQ(0xb2, buf[17]);
}
//-----------------------------------------------------------------------------
//Test for border value
TEST_F(SocksEncoderTest, CommandDomain255Symbols)
{
  VecByte buf;
  SocksCommandMsgResp msg;

  std::string domain
  {
    "11111111112222222222333333333344444444445555555555"
    "66666666667777777777888888888899999999990000000000"
    "11111111112222222222333333333344444444445555555555"
    "66666666667777777777888888888899999999990000000000"
    "11111111112222222222333333333344444444445555555555"
    "6.com"
  };

  ASSERT_EQ(255, domain.size());

  /*
    0x05,                                                   //version
    0x00,                                                   //status
    0x00,                                                   //reserved, must be 0x00
    0x03,                                                   //address type, domain
    0xff,                                                   //domain length
    "111111111112222222222333333333344444444445555555555"
    "666666666677777777777888888888899999999990000000000"
    "111111111122222222223333333333344444444445555555555"
    "666666666677777777778888888888999999999990000000000"
    "111111111122222222223333333333444444444455555555555"
    "6.com",                                                //domain
    0xa4, 0xb5                                              //port
  */

  msg._version._value = 0x05;
  msg._status = 0x00;
  msg._addrType = { SocksAddressType::DomainAddr };
  msg._addr = SocksDomainAddress{ { domain.begin(), domain.end() } };
  msg._port = 0xa4b5;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(262, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(0x00, buf[1]); //status
  EXPECT_EQ(0x00, buf[2]); //reserved
  EXPECT_EQ(SocksAddressType::DomainAddr, buf[3]);
  EXPECT_EQ(0xff, buf[4]);
  EXPECT_TRUE(std::equal(buf.begin() + 5, buf.begin() + 260, domain.begin()));
  EXPECT_EQ(0xa4, buf[260]);
  EXPECT_EQ(0xb5, buf[261]);
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, CommandIPv6Success)
{
  VecByte buf;
  SocksCommandMsgResp msg;

  /*
    0x05,                                                   //version
    0x00,                                                   //status
    0x00,                                                   //reserved, must be 0x00
    0x03,                                                   //address type, IPv6
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,         //IPv6
    0xa1, 0xb2                                              //port
  */

  msg._version._value = 0x05;
  msg._status = 0x00;
  msg._addrType = { SocksAddressType::IPv6Addr };
  SocksIPv6Address addr
  {{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
  }};
  msg._addr = addr;
  msg._port = 0xa2b3;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(22, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(0x00, buf[1]); //status
  EXPECT_EQ(0x00, buf[2]); //reserved
  EXPECT_EQ(SocksAddressType::IPv6Addr, buf[3]);
  EXPECT_EQ(0x00, buf[4]);
  EXPECT_EQ(0x01, buf[5]);
  EXPECT_EQ(0x02, buf[6]);
  EXPECT_EQ(0x03, buf[7]);
  EXPECT_EQ(0x04, buf[8]);
  EXPECT_EQ(0x05, buf[9]);
  EXPECT_EQ(0x06, buf[10]);
  EXPECT_EQ(0x07, buf[11]);
  EXPECT_EQ(0x08, buf[12]);
  EXPECT_EQ(0x09, buf[13]);
  EXPECT_EQ(0x0a, buf[14]);
  EXPECT_EQ(0x0b, buf[15]);
  EXPECT_EQ(0x0c, buf[16]);
  EXPECT_EQ(0x0d, buf[17]);
  EXPECT_EQ(0x0e, buf[18]);
  EXPECT_EQ(0x0f, buf[19]);
  EXPECT_EQ(0xa2, buf[20]);
  EXPECT_EQ(0xb3, buf[21]);
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, CommandTooLongDomain)
{
  //Domain can be from 1 to 255 symbols
  VecByte buf;
  SocksCommandMsgResp msg;

  /*
    0x05,                                                   //version
    0x00,                                                   //status
    0x00,                                                   //reserved, must be 0x00
    0x03,                                                   //address type, domain
    'verylongurlwithsomekindofstrangestrings.superpuperverylongdomain'
    '1111111111122222222223333333333444444444455555555556666666666'
    '7777777777788888888889999999999000000000011111111112222222222'
    '3333333333344444444445555555555666666666677777777778888888888'
    '9999999999900000000001111111111222222222233333333334444444444'
    '5555555555566666',                                     //domain
    0xa1, 0xb2                                              //port
  */

  SocksDomainAddress addr;
  std::string domain
  {
    "verylongurlwithsomekindofstrangestrings.superpuperverylongdomain"
    "1111111111122222222223333333333444444444455555555556666666666"
    "7777777777788888888889999999999000000000011111111112222222222"
    "3333333333344444444445555555555666666666677777777778888888888"
    "9999999999900000000001111111111222222222233333333334444444444"
    "5555555555566666"
  };
  addr._value = VecByte(domain.begin(), domain.end());

  msg._version._value = 0x05;
  msg._status = 0x00;
  msg._addrType = { SocksAddressType::DomainAddr };
  msg._addr = addr;
  msg._port = 0xa3b4;

  ASSERT_FALSE(_encoder->encode(msg, buf));
}
//-----------------------------------------------------------------------------
TEST_F(SocksEncoderTest, CommandIPv4ErrorStatus)
{
  VecByte buf;
  SocksCommandMsgResp msg;

  /*
    0x05,                   //version
    0x00,                   //status
    0x00,                   //reserved, must be 0x00
    0x01,                   //address type, IPv4
    0x00, 0x00, 0x00, 0x00, //IPv4
    0x00, 0x00              //port
  */

  msg._version._value = 0x05;
  msg._status = 0x01;
  msg._addrType = { SocksAddressType::IPv4Addr };
  msg._addr = SocksIPv4Address{ { 0x00, 0x00, 0x00, 0x00 } };
  msg._port = 0x0000;

  ASSERT_TRUE(_encoder->encode(msg, buf));
  ASSERT_EQ(10, buf.size());
  EXPECT_EQ(SocksVersion::Version5, buf[0]);
  EXPECT_EQ(SocksCommandMsgResp::GeneralFailure, buf[1]); //status
  EXPECT_EQ(0x00, buf[2]); //reserved
  EXPECT_EQ(SocksAddressType::IPv4Addr, buf[3]);
  EXPECT_EQ(0x00, buf[4]);
  EXPECT_EQ(0x00, buf[5]);
  EXPECT_EQ(0x00, buf[6]);
  EXPECT_EQ(0x00, buf[7]);
  EXPECT_EQ(0x00, buf[8]);
  EXPECT_EQ(0x00, buf[9]);
}
//-----------------------------------------------------------------------------
