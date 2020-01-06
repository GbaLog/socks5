#ifndef SocksTypesH
#define SocksTypesH
//-----------------------------------------------------------------------------
#include "Common.h"
#include <variant>
//-----------------------------------------------------------------------------
struct SocksVersion
{
  enum : Byte
  {
    Version4  = 0x04,
    Version4a = 0x04,
    Version5  = 0x05
  };
  Byte _value;
};
//-----------------------------------------------------------------------------
enum class SocksState
{
  Unauthorized,
  Authorization,
  WaitForConnectRequest,
  Connected
};
//-----------------------------------------------------------------------------
struct SocksAuthMethod
{
  enum : Byte
  {
    NoAuth = 0x00,
    AuthGSSAPI = 0x01,
    AuthLoginPass = 0x02
  };
  Byte _value;
};
using VecAuthMethod = std::vector<SocksAuthMethod>;
//-----------------------------------------------------------------------------
struct SocksCommandCode
{
  enum : Byte
  {
    TCPStream = 0x01,
    TCPPortBinding = 0x02,
    UDPPort = 0x03
  };
  Byte _value;
};
//-----------------------------------------------------------------------------
struct SocksAddressType
{
  enum : Byte
  {
    IPv4Addr = 0x01,
    DomainAddr = 0x03,
    IPv6Addr = 0x04
  };
  Byte _value;
};
//-----------------------------------------------------------------------------
struct SocksIPv4Address
{
  Byte _value[4];
};
//-----------------------------------------------------------------------------
struct SocksDomainAddress
{
  VecByte _value;
};
//-----------------------------------------------------------------------------
struct SocksIPv6Address
{
  Byte _value[16];
};
using SocksAddress = std::variant<SocksIPv4Address, SocksDomainAddress, SocksIPv6Address>;
//-----------------------------------------------------------------------------
struct SocksGreetingMsg
{
  SocksVersion _version;
  VecAuthMethod _authMethods;
};
//-----------------------------------------------------------------------------
struct SocksGreetingMsgResp
{
  SocksVersion _version;
  SocksAuthMethod _authMethod;
};
//-----------------------------------------------------------------------------
struct SocksUserPassAuthMsg
{
  SocksVersion _version;
  std::string _user;
  std::string _password;
};
//-----------------------------------------------------------------------------
struct SocksUserPassAuthMsgResp
{
  SocksVersion _version;
  Byte _status;
};
//-----------------------------------------------------------------------------
struct SocksConnReqMsg
{
  SocksVersion _version;
  SocksCommandCode _command;
  SocksAddressType _addrType;
  SocksAddress _addr;
  uint16_t _port; //In network byte order
};
//-----------------------------------------------------------------------------
struct SocksConnReqMsgResp
{
  enum : Byte
  {
    RequestGranted = 0x00,
    GeneralFailure = 0x01,
    RulesetFailure = 0x02, //Connection now allowed by ruleset
    NetworkUnreachable = 0x03,
    HostUnreachable = 0x04,
    ConnectionRefused = 0x05,
    TTLExpired = 0x06,
    CommandNotSupported = 0x07,
    AddressNotSupported = 0x08
  };

  SocksVersion _version;
  Byte _status;
  SocksAddress _addr;
  uint16_t _port; //In network byte order
};
//-----------------------------------------------------------------------------
#endif // SocksTypesH
//-----------------------------------------------------------------------------