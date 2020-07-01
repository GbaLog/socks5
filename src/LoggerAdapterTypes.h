#ifndef LoggerAdapterTypesH
#define LoggerAdapterTypesH
//-----------------------------------------------------------------------------
#include "Common.h"
#include "spdlog/fmt/ostr.h"
//-----------------------------------------------------------------------------
template<class OStream>
inline OStream & operator <<(OStream & strm, const VecByte & buf)
{
  auto flags = strm.flags();
  strm << "Size: " << buf.size() << ", data: ";
  strm << std::hex;
  for (const auto it : buf)
    strm << std::setw(2) << std::setfill('0') << (int)it << " ";
  strm << std::setfill(' ');
  strm.flags(flags);
  return strm;
}
//-----------------------------------------------------------------------------
#endif // LoggerAdapterTypesH
//-----------------------------------------------------------------------------
