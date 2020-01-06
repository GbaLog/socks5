#include "SocksDecoder.h"
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
bool isMethodExistV5(Byte val)
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
bool isMethodExist(SocksVersion version, Byte val)
{
  switch (version._value)
  {
  case SocksVersion::Version5:
    return isMethodExistV5(val);
  default:
    //TODO: Add other versions
    return false;
  }
}

bool isVersionSupport(Byte val)
{
  //TODO: Add other versions support
  switch (val)
  {
  case SocksVersion::Version5:
    return true;
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
} //namespace
//-----------------------------------------------------------------------------
bool SocksDecoder::decode(const VecByte & buf, SocksGreetingMsg & msg)
{
  if (buf.size() < 2)
    return false;

  if (isVersionSupport(buf[0]) == false)
    return false;

  msg._version._value = buf[0];
  Byte numberOfMethods = buf[1];
  if (buf.size() != numberOfMethods + 2)
    return false;

  msg._authMethods.reserve(numberOfMethods);
  for (size_t i = 2; i < buf.size(); ++i)
  {
    if (isMethodExist(msg._version, buf[i]) == false)
      return false;

    msg._authMethods.push_back({ buf[i] });
  }
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::decode(const VecByte & buf, SocksUserPassAuthMsg & msg)
{
  if (buf.size() < 3)
    return false;

  if (isVersionSupport(buf[0]) == false)
    return false;

  msg._version._value = buf[0];
  Byte userLen = buf[1];
  if (buf.size() < userLen + 3)
    return false;
  msg._user.reserve(userLen);
  std::copy(buf.data() + 2, buf.data() + 2 + userLen, std::back_inserter(msg._user));

  Byte passLen = buf[2 + userLen];
  if (buf.size() != userLen + passLen + 3)
    return false;
  msg._password.reserve(passLen);
  std::copy(buf.data() + 3 + userLen, buf.data() + 3 + userLen + passLen, std::back_inserter(msg._password));
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::decode(const VecByte & buf, SocksConnReqMsg & msg)
{
  return false;
}
//-----------------------------------------------------------------------------
