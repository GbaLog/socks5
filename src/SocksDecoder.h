#ifndef SocksDecoderH
#define SocksDecoderH
//-----------------------------------------------------------------------------
#include "SocksTypes.h"
#include "Common.h"
#include <memory>
//-----------------------------------------------------------------------------
class SocksDecoder
{
public:
  SocksDecoder(SocksVersion version);
  ~SocksDecoder();

  bool decode(const VecByte & buf, SocksGreetingMsg & msg);
  bool decode(const VecByte & buf, SocksUserPassAuthMsg & msg);
  bool decode(const VecByte & buf, SocksConnReqMsg & msg);

private:
  std::unique_ptr<class SocksDecoderImpl> _impl;
};
//-----------------------------------------------------------------------------
#endif // SocksDecoderH
//-----------------------------------------------------------------------------
