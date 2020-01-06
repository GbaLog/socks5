#ifndef RatelInetUtilsH
#define RatelInetUtilsH
//-----------------------------------------------------------------------------
#include <cstdint>
#include <iosfwd>
//-----------------------------------------------------------------------------
namespace ratel
{
#if defined(_MSC_VER)
#define STRUCT_PACK_BEGIN __pragma(pack(push, 1))
#define STRUCT_PACK_END __pragma(pack(pop))
#elif defined(__MINGW32__) || defined(__GNUC__)
#define STRUCT_PACK_BEGIN _Pragma("pack(push, 1)")
#define STRUCT_PACK_END _Pragma("pack(pop)")
#else
#error "Unsupported platform"
#endif
//-----------------------------------------------------------------------------
uint16_t htons(uint16_t val);
uint32_t htonl(uint32_t val);
//-----------------------------------------------------------------------------
} //namespace ratel
//-----------------------------------------------------------------------------
struct IpAddressAndPort
{
  uint32_t _ip;
  uint16_t _port;
};
//-----------------------------------------------------------------------------
IpAddressAndPort convertToAddr(struct sockaddr_in addr);
//-----------------------------------------------------------------------------
std::ostream & operator <<(std::ostream & strm, const IpAddressAndPort & addr);
//-----------------------------------------------------------------------------
#endif // RatelInetUtilsH
