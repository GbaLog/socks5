#ifndef EventSocketCommonH
#define EventSocketCommonH
//-----------------------------------------------------------------------------
#include <memory>
#include <string_view>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include "SocksTypes.h"
//-----------------------------------------------------------------------------
typedef std::shared_ptr<event_base> EventBasePtr;
typedef std::unique_ptr<bufferevent, void (*) (bufferevent *)> BufferEventPtr;
typedef std::unique_ptr<evconnlistener, void (*)(evconnlistener *)> EventListenerPtr;
typedef std::unique_ptr<event, void (*) (event *)> EventPtr;
//-----------------------------------------------------------------------------
BufferEventPtr makeBufferEvent(EventBasePtr base, evutil_socket_t fd);
//-----------------------------------------------------------------------------
EventListenerPtr makeListener(EventBasePtr base, evconnlistener_cb cb, void * self,
                              const sockaddr * saddr, int salen);
//-----------------------------------------------------------------------------
std::string_view getAddrStr(const sockaddr * addr);
//-----------------------------------------------------------------------------
uint16_t getAddrPort(const sockaddr * addr);
//-----------------------------------------------------------------------------
bool convertAddrToStorage(const SocksAddress & addr, sockaddr_storage * sa);
//-----------------------------------------------------------------------------
SocksAddress getRemoteSocketAddress(evutil_socket_t fd);
SocksAddress getLocalSocketAddress(evutil_socket_t fd);
//-----------------------------------------------------------------------------
#endif //EventSocketCommonH
//-----------------------------------------------------------------------------
