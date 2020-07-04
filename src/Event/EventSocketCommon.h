#ifndef EventSocketCommonH
#define EventSocketCommonH
//-----------------------------------------------------------------------------
#include <memory>
#include <string_view>
#include <event2/event.h>
//-----------------------------------------------------------------------------
typedef std::shared_ptr<event_base> EventBasePtr;
//-----------------------------------------------------------------------------
std::string_view getAddrStr(sockaddr * addr);
//-----------------------------------------------------------------------------
uint16_t getAddrPort(sockaddr * addr);
//-----------------------------------------------------------------------------
#endif //EventSocketCommonH
//-----------------------------------------------------------------------------
