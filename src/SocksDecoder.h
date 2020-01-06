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

  bool decode(const VecByte & buf, SocksGreetingMsg & msg) const;
  bool decode(const VecByte & buf, SocksUserPassAuthMsg & msg) const;
  bool decode(const VecByte & buf, SocksCommandMsg & msg) const;

private:
  class SocksDecoderImpl;
  std::unique_ptr<SocksDecoderImpl> _impl;
};
//-----------------------------------------------------------------------------
#endif // SocksDecoderH
//-----------------------------------------------------------------------------
