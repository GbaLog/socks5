#ifndef RatelCommonH
#define RatelCommonH
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <iomanip>
//-----------------------------------------------------------------------------
typedef unsigned char Byte;
typedef std::vector<Byte> VecByte;
typedef std::vector<std::string> VecString;

class NonCopyable
{
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable & operator =(const NonCopyable &) = delete;
};

inline std::ostream & operator <<(std::ostream & strm, const VecByte & buf)
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
#endif // RatelCommonH
