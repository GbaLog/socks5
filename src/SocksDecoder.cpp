#include "SocksDecoder.h"
#include "SocksCommon.h"
#include <cstring>
//-----------------------------------------------------------------------------
class SocksDecoder::SocksDecoderImpl
{
public:
  SocksDecoderImpl();
  ~SocksDecoderImpl() = default;

  bool decode(const VecByte & buf, SocksGreetingMsg & msg) const;
  bool decode(const VecByte & buf, SocksUserPassAuthMsg & msg) const;
  bool decode(const VecByte & buf, SocksCommandMsg & msg) const;

private:
  SocksVersion _version;

  bool decodeIPv4(const VecByte & buf, SocksCommandMsg & msg) const;
  bool decodeDomain(const VecByte & buf, SocksCommandMsg & msg) const;
  bool decodeIPv6(const VecByte & buf, SocksCommandMsg & msg) const;
  bool decodePort(const VecByte & buf, SocksCommandMsg & msg) const;
};
//-----------------------------------------------------------------------------
SocksDecoder::SocksDecoderImpl::SocksDecoderImpl() :
  _version(SocksVersion{SocksVersion::Version5})
{}
//-----------------------------------------------------------------------------
bool SocksDecoder::SocksDecoderImpl::decode(const VecByte & buf, SocksGreetingMsg & msg) const
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
    if (isAuthMethodExist(buf[i]) == false)
      return false;

    msg._authMethods.push_back({ buf[i] });
  }
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::SocksDecoderImpl::decode(const VecByte & buf, SocksUserPassAuthMsg & msg) const
{
  if (buf.size() < 3)
    return false;

  if (buf[0] != 0x01)
    return false;

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
bool SocksDecoder::SocksDecoderImpl::decode(const VecByte & buf, SocksCommandMsg & msg) const
{
  if (buf.size() < 6)
    return false;

  if (isVersionSupport(buf[0]) == false)
    return false;
  if (buf[2] != 0x00) //reserved
    return false;

  msg._version._value = buf[0];

  if (isCommandExist(buf[1]) == false)
    return false;
  msg._command._value = buf[1];

  if (isAddressTypeExist(buf[3]) == false)
    return false;
  msg._addrType._value = buf[3];
  switch (msg._addrType._value)
  {
  case SocksAddressType::IPv4Addr:
    if (decodeIPv4(buf, msg) == false)
      return false;
    break;
  case SocksAddressType::DomainAddr:
    if (decodeDomain(buf, msg) == false)
      return false;
    break;
  case SocksAddressType::IPv6Addr:
    if (decodeIPv6(buf, msg) == false)
      return false;
    break;
  default:
    return false;
  }

  if (decodePort(buf, msg) == false)
    return false;
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::SocksDecoderImpl::decodeIPv4(const VecByte & buf, SocksCommandMsg & msg) const
{
  if (buf.size() < 8)
    return false;
  SocksIPv4Address addr;
  uint32_t ip = *(uint32_t *)(&buf[4]);
  memcpy(&addr, &ip, sizeof(ip));
  msg._addr = std::move(addr);
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::SocksDecoderImpl::decodeDomain(const VecByte & buf, SocksCommandMsg & msg) const
{
  if (buf.size() < 5)
    return false;
  Byte domainLen = buf[4];
  if (buf.size() < 5 + domainLen)
    return false;
  SocksDomainAddress addr;
  addr._value.reserve(domainLen);
  std::copy(buf.data() + 5, buf.data() + 5 + domainLen, std::back_inserter(addr._value));
  msg._addr = std::move(addr);
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::SocksDecoderImpl::decodeIPv6(const VecByte & buf, SocksCommandMsg & msg) const
{
  if (buf.size() < 20)
    return false;
  SocksIPv6Address addr;
  std::copy(buf.data() + 4, buf.data() + 20, addr._value);
  msg._addr = std::move(addr);
  return true;
}
//-----------------------------------------------------------------------------
bool SocksDecoder::SocksDecoderImpl::decodePort(const VecByte & buf, SocksCommandMsg & msg) const
{
  switch (msg._addrType._value)
  {
  case SocksAddressType::IPv4Addr:
    if (buf.size() != 10)
      return false;
    memcpy(&msg._port, &buf[8], 2);
    return true;
  case SocksAddressType::DomainAddr:
  {
    SocksDomainAddress & addr = std::get<SocksDomainAddress>(msg._addr);
    if (buf.size() != 7 + addr._value.size())
      return false;
    memcpy(&msg._port, &buf[5 + addr._value.size()], 2);
    return true;
  }
  case SocksAddressType::IPv6Addr:
    if (buf.size() != 22)
      return false;
    memcpy(&msg._port, &buf[20], 2);
    return true;
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
SocksDecoder::SocksDecoder() :
  _impl(std::make_unique<SocksDecoderImpl>())
{}
//-----------------------------------------------------------------------------
SocksDecoder::~SocksDecoder() = default;
//-----------------------------------------------------------------------------
bool SocksDecoder::decode(const VecByte & buf, SocksGreetingMsg & msg) const
{
  return _impl->decode(buf, msg);
}
//-----------------------------------------------------------------------------
bool SocksDecoder::decode(const VecByte & buf, SocksUserPassAuthMsg & msg) const
{
  return _impl->decode(buf, msg);
}
//-----------------------------------------------------------------------------
bool SocksDecoder::decode(const VecByte & buf, SocksCommandMsg & msg) const
{
  return _impl->decode(buf, msg);
}
//-----------------------------------------------------------------------------
