#include "EventSocketCommon.h"
//-----------------------------------------------------------------------------
#ifdef _WIN32
#include <ws2ipdef.h>
#endif //WIN32
//-----------------------------------------------------------------------------
std::string_view getAddrStr(sockaddr * addr)
{
  static thread_local char buf[46];
  if (addr->sa_family == AF_INET6)
  {
    sockaddr_in6 * in6 = (sockaddr_in6 *)addr;
    return evutil_inet_ntop(AF_INET6, &in6->sin6_addr, buf, sizeof(buf));
  }
  sockaddr_in * in = (sockaddr_in *)addr;
  return evutil_inet_ntop(AF_INET, &in->sin_addr, buf, sizeof(buf));
}
//-----------------------------------------------------------------------------
uint16_t getAddrPort(sockaddr * addr)
{
  if (addr->sa_family == AF_INET6)
  {
    sockaddr_in6 * in6 = (sockaddr_in6 *)addr;
    return ntohs(in6->sin6_port);
  }

  sockaddr_in * in = (sockaddr_in *)addr;
  return ntohs(in->sin_port);
}
//-----------------------------------------------------------------------------
