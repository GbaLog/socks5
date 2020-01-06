#ifndef SocksDecoderH
#define SocksDecoderH
//-----------------------------------------------------------------------------
#include "SocksTypes.h"
#include "Common.h"
//-----------------------------------------------------------------------------
class SocksDecoder : NonCopyable
{
public:
  SocksDecoder() = delete;

  static bool decode(const VecByte & buf, SocksGreetingMsg & msg);
  static bool decode(const VecByte & buf, SocksUserPassAuthMsg & msg);
  static bool decode(const VecByte & buf, SocksConnReqMsg & msg);
};
//-----------------------------------------------------------------------------
#endif // SocksDecoderH
//-----------------------------------------------------------------------------
