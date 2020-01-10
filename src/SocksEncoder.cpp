#include "SocksEncoder.h"
#include "SocksCommon.h"

class SocksEncoder::SocksEncoderImpl
{
public:
  SocksEncoderImpl(SocksVersion version);

  bool encode(const SocksGreetingMsgResp & msg, VecByte & buf) const;
  bool encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf) const;
  bool encode(const SocksCommandMsgResp & msg, VecByte & buf) const;

private:
  SocksVersion _version;
};

SocksEncoder::SocksEncoderImpl::SocksEncoderImpl(SocksVersion version) :
  _version(version)
{}

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
      buf[8] = msg._port >> 8;
      buf[9] = msg._port & 0x00ff;
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
      buf[5 + addr._value.size()] = msg._port >> 8;
      buf[6 + addr._value.size()] = msg._port & 0x00ff;
      break;
    }
  case SocksAddressType::IPv6Addr:
    {
      const SocksIPv6Address & addr = std::get<SocksIPv6Address>(msg._addr);
      buf.resize(22);
      std::copy(std::begin(addr._value), std::end(addr._value), buf.data() + 4);
      buf[20] = msg._port >> 8;
      buf[21] = msg._port & 0x00ff;
      break;
    }
  }

  return true;
}

SocksEncoder::SocksEncoder(SocksVersion version) :
  _impl(std::make_unique<SocksEncoderImpl>(version))
{}

SocksEncoder::~SocksEncoder() = default;

bool SocksEncoder::encode(const SocksGreetingMsgResp & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}

bool SocksEncoder::encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}

bool SocksEncoder::encode(const SocksCommandMsgResp & msg, VecByte & buf)
{
  return _impl->encode(msg, buf);
}
