#include "SocksCommon.h"
#include "SocksTypes.h"
//-----------------------------------------------------------------------------
//TODO: Add support of Socks4, Socks4a
//-----------------------------------------------------------------------------
bool isVersionSupport(Byte val)
{
  switch (val)
  {
  case SocksVersion::Version5:
    return true;
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
bool isAuthMethodExist(Byte val)
{
  switch (val)
  {
  case SocksAuthMethod::NoAuth:
  case SocksAuthMethod::AuthGSSAPI:
  case SocksAuthMethod::AuthLoginPass:
    return true;
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
bool isCommandExist(Byte val)
{
  switch (val)
  {
  case SocksCommandCode::TCPPortBinding:
  case SocksCommandCode::TCPStream:
  case SocksCommandCode::UDPPortBinding:
    return true;
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
bool isAddressTypeExist(Byte val)
{
  switch (val)
  {
  case SocksAddressType::IPv4Addr:
  case SocksAddressType::DomainAddr:
  case SocksAddressType::IPv6Addr:
    return true;
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
