#include "SocksEncoder.h"

bool SocksEncoder::encode(const SocksGreetingMsgResp & msg, VecByte & buf)
{
  return false;
}

bool SocksEncoder::encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf)
{
  return false;
}

bool SocksEncoder::encode(const SocksConnReqMsgResp & msg, VecByte & buf)
{
  return false;
}
