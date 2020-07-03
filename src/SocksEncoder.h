#ifndef SocksEncoderH
#define SocksEncoderH
//-----------------------------------------------------------------------------
#include "Common.h"
#include "SocksTypes.h"
#include <memory>
//-----------------------------------------------------------------------------
class SocksEncoder
{
public:
  SocksEncoder();
  ~SocksEncoder();

  bool encode(const SocksGreetingMsg & msg, VecByte & buf);
  bool encode(const SocksGreetingMsgResp & msg, VecByte & buf);
  bool encode(const SocksUserPassAuthMsg & msg, VecByte & buf);
  bool encode(const SocksUserPassAuthMsgResp & msg, VecByte & buf);
  bool encode(const SocksCommandMsg & msg, VecByte & buf);
  bool encode(const SocksCommandMsgResp & msg, VecByte & buf);

private:
  class SocksEncoderImpl;
  std::unique_ptr<SocksEncoderImpl> _impl;
};
//-----------------------------------------------------------------------------
#endif // SocksEncoderH
//-----------------------------------------------------------------------------
