#include "EventSocketCommon.h"
#include "LoggerAdapter.h"
//-----------------------------------------------------------------------------
#ifdef _WIN32
#include <ws2ipdef.h>

typedef int socklen_t;
#else // _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // _WIN32
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
SocksAddress getAddress(const sockaddr * sa, socklen_t salen)
{
  SocksAddress addr;

  switch (sa->sa_family)
  {
  case AF_INET:
    {
      addr._type._value = SocksAddressType::IPv4Addr;

      sockaddr_in * in = (sockaddr_in *)sa;
      in->sin_addr.s_addr = ntohl(in->sin_addr.s_addr); // To native byte-order
      SocksIPv4Address ip4Addr;
      ::memcpy(&ip4Addr._value, &in->sin_addr.s_addr, sizeof(ip4Addr._value));
      addr._addr = std::move(ip4Addr);
      addr._port = ntohs(in->sin_port);
      break;
    }
  case AF_INET6:
    {
      addr._type._value = SocksAddressType::IPv6Addr;

      sockaddr_in6 * in6 = (sockaddr_in6 *)sa;
      SocksIPv6Address ip6Addr;
      ::memcpy(&ip6Addr._value, &in6->sin6_addr.s6_addr, sizeof(ip6Addr._value));
      addr._addr = std::move(ip6Addr);
      addr._port = ntohs(in6->sin6_port);
      break;
    }
  default:
    {
      LoggerAdapter::logSingle(CRIT, "Unknown socker address family: {}", sa->sa_family);
      assert(false && !"Unknown socket address family");
      break;
    }
  }

  return addr;
}
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
BufferEventPtr makeBufferEvent(EventBasePtr base, evutil_socket_t fd)
{
  bufferevent * bev = bufferevent_socket_new(base.get(), fd, BEV_OPT_CLOSE_ON_FREE);
  if (bev == nullptr)
    return BufferEventPtr{nullptr, nullptr};
  return BufferEventPtr{bev, bufferevent_free};
}
//-----------------------------------------------------------------------------
EventListenerPtr makeListener(EventBasePtr base, evconnlistener_cb cb, void * self,
                              const sockaddr * saddr, int salen)
{
  evconnlistener * listener = evconnlistener_new_bind(base.get(), cb, self,
                                                      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                                      saddr, salen);
  if (listener == nullptr)
    return EventListenerPtr{nullptr, nullptr};
  return EventListenerPtr{listener, evconnlistener_free};
}
//-----------------------------------------------------------------------------
std::string_view getAddrStr(const sockaddr * addr)
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
uint16_t getAddrPort(const sockaddr * addr)
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
bool convertAddrToStorage(const SocksAddress & addr, sockaddr_storage * sa)
{
  switch (addr._type._value)
  {
  case SocksAddressType::IPv4Addr:
    {
      const SocksIPv4Address & ip4 = std::get<SocksIPv4Address>(addr._addr);
      auto * in = (sockaddr_in *)sa;
      in->sin_family = AF_INET;
      ::memcpy(&in->sin_addr.s_addr, ip4._value, sizeof(in->sin_addr.s_addr));
      in->sin_port = addr._port;
      return true;
    }
  case SocksAddressType::IPv6Addr:
    {
      const SocksIPv6Address & ip6 = std::get<SocksIPv6Address>(addr._addr);
      auto * in6 = (sockaddr_in6 *)sa;
      in6->sin6_family = AF_INET6;
      ::memcpy(&in6->sin6_addr.s6_addr, ip6._value, sizeof(in6->sin6_addr.s6_addr));
      in6->sin6_port = addr._port;
      return true;
    }
  default:
    return false;
  }
}
//-----------------------------------------------------------------------------
SocksAddress getRemoteSocketAddress(evutil_socket_t fd)
{
  sockaddr_storage sa;
  memset(&sa, 0, sizeof(sa));
  socklen_t salen = sizeof(sa);

  getpeername(fd, (sockaddr *)&sa, &salen);
  return getAddress(reinterpret_cast<const sockaddr *>(&sa), salen);
}
//-----------------------------------------------------------------------------
SocksAddress getLocalSocketAddress(evutil_socket_t fd)
{
  sockaddr_storage sa;
  memset(&sa, 0, sizeof(sa));
  socklen_t salen = sizeof(sa);

  getsockname(fd, (sockaddr *)&sa, &salen);
  return getAddress(reinterpret_cast<const sockaddr *>(&sa), salen);
}
//-----------------------------------------------------------------------------
