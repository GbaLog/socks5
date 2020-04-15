#include "InetUtils.h"
#include <sstream>
//-----------------------------------------------------------------------------
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
//-----------------------------------------------------------------------------
namespace ratel
{
//-----------------------------------------------------------------------------
uint16_t htons(uint16_t val)
{
  return ::htons(val);
}
//-----------------------------------------------------------------------------
uint32_t htonl(uint32_t val)
{
  return ::htonl(val);
}
//-----------------------------------------------------------------------------
uint16_t ntohs(uint16_t val)
{
  return ::ntohs(val);
}
//-----------------------------------------------------------------------------
uint32_t ntohl(uint32_t val)
{
  return ::ntohl(val);
}
//-----------------------------------------------------------------------------
uint32_t inet_addr(const char * addr)
{
  return ::inet_addr(addr);
}
//-----------------------------------------------------------------------------
} //namespace ratel
//-----------------------------------------------------------------------------
IpAddressAndPort convertToAddr(sockaddr_in addr)
{
  IpAddressAndPort ret;
  ret._ip = addr.sin_addr.s_addr;
  ret._port = addr.sin_port;
  return ret;
}
//-----------------------------------------------------------------------------
std::ostream & operator <<(std::ostream & strm, const IpAddressAndPort & addr)
{
  in_addr tmpAddr;
  tmpAddr.s_addr = addr._ip;
  return (strm << "IP: " << inet_ntoa(tmpAddr) << ", port: " << htons(addr._port));
}
//-----------------------------------------------------------------------------
