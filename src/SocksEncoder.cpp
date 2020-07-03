#include "SocksEncoder.h"
#include "SocksCommon.h"
#include <cstring>
//-----------------------------------------------------------------------------
class SocksEncoder::SocksEncoderImpl
{
public:
  SocksEncoderImpl(SocksVersion version);

  bool encode(const SocksGreetingMsg & msg, VecByte & buf) const;
  bool encode(const SocksGreetingMsgResp & msg, VecByte & buf) const;
  bool encode(const SocksUserPassAuthMsg & msg, VecByte & buf) const;
  bool encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf) const;
  bool encode(const SocksCommandMsg & msg, VecByte & buf) const;
  bool encode(const SocksCommandMsgResp & msg, VecByte & buf) const;

private:
  SocksVersion _version;
};
//-----------------------------------------------------------------------------
SocksEncoder::SocksEncoderImpl::SocksEncoderImpl(SocksVersion version) :
  _version(version)
{}
//-----------------------------------------------------------------------------
bool SocksEncoder::SocksEncoderImpl::encode(const SocksGreetingMsg & msg, VecByte & buf) const
{
  if (msg._version._value != _version._value ||
      isVersionSupport(msg._version._value) == false)
    return false;

  if (msg._authMethods.empty())
    return false;

  for (const auto it : msg._authMethods)
  {
    if (isAuthMethodExist(it._value) == false &&
        it._value != SocksAuthMethod::NoAvailableMethod)
    {
      return false;
    }
  }

  buf.resize(2 + msg._authMethods.size());
  buf[0] = msg._version._value;
  buf[1] = (uint8_t)msg._authMethods.size();
  for (size_t i = 0; i < msg._authMethods.size(); ++i)
    buf[i + 2] = msg._authMethods[i]._value;
  return true;
}
//-----------------------------------------------------------------------------
bool SocksEncoder::SocksEncoderImpl::encode(const SocksGreetingMsgResp & msg, VecByte & buf) const
{
  if (msg._version._value != _version._value ||
      isVersionSupport(msg._version._value) == false)
    return false;

  if (isAuthMethodExist(msg._authMethod._value) == false &&
      msg._authMethod._value != SocksAuthMethod::NoAvailableMethod)
    return false;

  buf.resize(2);
  buf[0] = msg._version._value;
  buf[1] = msg._authMethod._value;
  return true;
}
//-----------------------------------------------------------------------------
bool SocksEncoder::SocksEncoderImpl::encode(const SocksUserPassAuthMsg & msg, VecByte & buf) const
{
  auto wrongString = [] (const std::string & str)
  {
    return str.empty() || str.size() > 255;
  };

  if (wrongString(msg._user)|| wrongString(msg._password))
    return false;
  buf.resize(msg._user.size() + msg._password.size() + 3);
  buf[0] = 0x01;
  buf[1] = (uint8_t)msg._user.size();
  ::memcpy(&buf[2], msg._user.data(), msg._user.size());
  buf[msg._user.size() + 2] = (uint8_t)msg._password.size();
  ::memcpy(&buf[msg._user.size() + 3], msg._password.data(), msg._password.size());
  return true;
}
//-----------------------------------------------------------------------------
bool SocksEncoder::SocksEncoderImpl::encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf) const
{
  if (msg._version._value != _version._value ||
      isVersionSupport(msg._version._value) == false)
    return false;

  buf.resize(2);
  buf[0] = msg._version._value;
  buf[1] = msg._status;
  return true;
}
//-----------------------------------------------------------------------------
bool SocksEncoder::SocksEncoderImpl::encode(const SocksCommandMsg & msg, VecByte & buf) const
{
  if (msg._version._value != _version._value)
    return false;

  if (isAddressTypeExist(msg._addrType._value) == false)
    return false;

  if (isCommandExist(msg._command._value) == false)
    return false;

  buf.resize(6);
  buf[0] = msg._version._value;
  buf[1] = msg._command._value;
  buf[2] = 0x00;
  buf[3] = msg._addrType._value;

  switch (msg._addrType._value)
  {
  case SocksAddressType::IPv4Addr:
    {
      const SocksIPv4Address & addr = std::get<SocksIPv4Address>(msg._addr);
      buf.resize(10);
      ::memcpy(buf.data() + 4, addr._value, 4);
      break;
    }
  case SocksAddressType::DomainAddr:
    {
      const SocksDomainAddress & addr = std::get<SocksDomainAddress>(msg._addr);
      buf.resize(7 + addr._value.size());
      buf[4] = (uint8_t)addr._value.size();
      ::memcpy(buf.data() + 5, addr._value.data(), addr._value.size());
      break;
    }
  case SocksAddressType::IPv6Addr:
    {
      const SocksIPv6Address & addr = std::get<SocksIPv6Address>(msg._addr);
      buf.resize(22);
      ::memcpy(buf.data() + 4, addr._value, 16);
      break;
    }
  default:
    return false;
  }

  ::memcpy(buf.data() + buf.size() - 2, &msg._port, 2);
  return true;
}
//-----------------------------------------------------------------------------
bool SocksEncoder::SocksEncoderImpl::encode(const SocksCommandMsgResp & msg, VecByte & buf) const
{
  if (msg._version._value != _version._value ||
      isVersionSupport(msg._version._value) == false)
    return false;

  if (isAddressTypeExist(msg._addrType._value) == false)
    return false;

  buf.resize(6);
  buf[0] = msg._version._value;
  buf[1] = msg._status;
  buf[2] = 0x00; //reserved;
  buf[3] = msg._addrType._value;

  switch (msg._addrType._value)
  {
  case SocksAddressType::IPv4Addr:
    {
      const SocksIPv4Address & addr = std::get<SocksIPv4Address>(msg._addr);
      buf.resize(10);
      std::copy(std::begin(addr._value), std::end(addr._value), buf.data() + 4);
      memcpy(&buf[8], &msg._port, 2);
      break;
    }
  case SocksAddressType::DomainAddr:
    {
      const SocksDomainAddress & addr = std::get<SocksDomainAddress>(msg._addr);
      if (addr._value.size() > 255)
        return false;
      buf.resize(7 + addr._value.size());
      buf[4] = addr._value.size();
      std::copy(std::begin(addr._value), std::end(addr._value), buf.data() + 5);
      memcpy(&buf[5 + addr._value.size()], &msg._port, 2);
      break;
    }
  case SocksAddressType::IPv6Addr:
    {
      const SocksIPv6Address & addr = std::get<SocksIPv6Address>(msg._addr);
      buf.resize(22);
      std::copy(std::begin(addr._value), std::end(addr._value), buf.data() + 4);
      memcpy(&buf[20], &msg._port, 2);
      break;
    }
  }

  return true;
}
//-----------------------------------------------------------------------------
SocksEncoder::SocksEncoder(SocksVersion version) :
  _impl(std::make_unique<SocksEncoderImpl>(version))
{}
//-----------------------------------------------------------------------------
SocksEncoder::~SocksEncoder() = default;
//-----------------------------------------------------------------------------
bool SocksEncoder::encode(const SocksGreetingMsg & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
//-----------------------------------------------------------------------------
bool SocksEncoder::encode(const SocksGreetingMsgResp & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
//-----------------------------------------------------------------------------
bool SocksEncoder::encode(const SocksUserPassAuthMsg & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
//-----------------------------------------------------------------------------
bool SocksEncoder::encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
//-----------------------------------------------------------------------------
bool SocksEncoder::encode(const SocksCommandMsg & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
//-----------------------------------------------------------------------------
bool SocksEncoder::encode(const SocksCommandMsgResp & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
//-----------------------------------------------------------------------------
