#ifndef SocksEncoderH
#define SocksEncoderH
//-----------------------------------------------------------------------------
#include "Common.h"
#include "SocksTypes.h"
//-----------------------------------------------------------------------------
class SocksEncoder : NonCopyable
{
public:
  SocksEncoder() = delete;

  static bool encode(const SocksGreetingMsgResp & msg, VecByte & buf);
  static bool encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf);
  static bool encode(const SocksConnReqMsgResp & msg, VecByte & buf);
};
//-----------------------------------------------------------------------------
#endif // SocksEncoderH
//-----------------------------------------------------------------------------
