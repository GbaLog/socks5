#ifndef SocksTypesH
#define SocksTypesH
//-----------------------------------------------------------------------------
#include "Common.h"
#include <variant>
#include <optional>
#include <cstring>
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
    AuthLoginPass = 0x02,
    NoAvailableMethod = 0xff
  };
  Byte _value;
};
using VecAuthMethod = std::vector<SocksAuthMethod>;

constexpr bool operator ==(const SocksAuthMethod & lhs, const SocksAuthMethod & rhs)
{ return lhs._value == rhs._value; }
//-----------------------------------------------------------------------------
struct SocksCommandCode
{
  enum : Byte
  {
    TCPStream = 0x01,
    TCPPortBinding = 0x02,
    UDPPortBinding = 0x03
  };
  Byte _value;
};
//-----------------------------------------------------------------------------
constexpr bool operator ==(const SocksCommandCode & lhs, const SocksCommandCode & rhs)
{ return lhs._value == rhs._value; }
//-----------------------------------------------------------------------------
using SocksPort = uint16_t;
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
constexpr bool operator ==(const SocksIPv4Address & lhs, const SocksIPv4Address & rhs)
{ return ::memcmp(lhs._value, rhs._value, sizeof(lhs._value)) == 0; }
//-----------------------------------------------------------------------------
struct SocksDomainAddress
{
  VecByte _value;
};
//-----------------------------------------------------------------------------
inline bool operator ==(const SocksDomainAddress & lhs, const SocksDomainAddress & rhs)
{
  return lhs._value.size() == rhs._value.size() &&
         ::memcmp(lhs._value.data(), rhs._value.data(), lhs._value.size()) == 0;
}
//-----------------------------------------------------------------------------
struct SocksIPv6Address
{
  Byte _value[16];
};
//-----------------------------------------------------------------------------
constexpr bool operator ==(const SocksIPv6Address & lhs, const SocksIPv6Address & rhs)
{ return ::memcmp(lhs._value, rhs._value, sizeof(lhs._value)) == 0; }
//-----------------------------------------------------------------------------
using SocksVariantAddress = std::variant<SocksIPv4Address, SocksDomainAddress, SocksIPv6Address>;
//-----------------------------------------------------------------------------
struct SocksAddress
{
  SocksAddressType _type;
  SocksVariantAddress _addr;
  SocksPort _port;
};
//-----------------------------------------------------------------------------
constexpr bool operator ==(const SocksAddress & lhs, const SocksAddress & rhs)
{
  return std::tie(lhs._type._value, lhs._addr, lhs._port) ==
         std::tie(rhs._type._value, rhs._addr, rhs._port);
}
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
struct SocksCommandMsg
{
  SocksVersion _version;
  SocksCommandCode _command;
  SocksAddressType _addrType;
  SocksVariantAddress _addr;
  SocksPort _port; //In network byte order
};
//-----------------------------------------------------------------------------
struct SocksCommandMsgResp
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
  SocksAddressType _addrType;
  SocksVariantAddress _addr;
  SocksPort _port; //In network byte order
};
//-----------------------------------------------------------------------------
#endif // SocksTypesH
//-----------------------------------------------------------------------------
