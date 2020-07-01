#ifndef RatelCommonH
#define RatelCommonH
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <iomanip>
#include "CpuConfig.h"
//-----------------------------------------------------------------------------
typedef unsigned char Byte;
typedef std::vector<Byte> VecByte;
typedef std::vector<std::string> VecString;
//-----------------------------------------------------------------------------
class NonCopyable
{
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable & operator =(const NonCopyable &) = delete;
};
//-----------------------------------------------------------------------------
#endif // RatelCommonH
