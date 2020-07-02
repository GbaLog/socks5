#include <cassert>
#include <cstring>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include "SocksSessionMng.h"
#include "InetUtils.h"
#include "LoggerAdapter.h"

extern "C"
void eventLog(int severity, const char * msg)
{
  int lvl = ERR;
  switch (severity)
  {
  case EVENT_LOG_DEBUG: lvl = DBG; break;
  case EVENT_LOG_MSG:   lvl = INF; break;
  case EVENT_LOG_WARN:  lvl = WRN; break;
  case EVENT_LOG_ERR:   lvl = ERR; break;
  default:              lvl = ERR; break;
  }

  LoggerAdapter::logSingle(lvl, "[EvLog] {}", msg);
}

static
bool initLogging()
{
  std::string appName = "socks5";
  int maxBytes = 1024 * 1024 * 10; //10 MByte
  int maxFiles = 10;

  try
  {
    LoggerAdapter::globalInit(appName, maxBytes, maxFiles);
  }
  catch (const spdlog::spdlog_ex & ex)
  {
    spdlog::error("Exception while initiating logger: {}", ex.what());
    return false;
  }
  return true;
}

int main(int argc, char * argv[])
{
  if (initLogging() == false)
    return EXIT_FAILURE;

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
  {
    LoggerAdapter::logSingle(INF, "Usage: {} [host] [port]", argv[0]);
    return 0;
  }

  uint32_t hostIP = 0;
  uint16_t hostPort = ratel::htons(35555);
  
  if (argc >= 2)
  {
    auto tmpAddr = ratel::inet_addr(argv[1]);
    hostIP = tmpAddr;
  }
  if (argc >= 3)
  {
    hostPort = ratel::htons(std::stoi(argv[2]));
  }

#ifdef _WIN32
  WSADATA wsaData;
  assert(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
#endif

  event_set_log_callback(eventLog);
  event_enable_debug_logging(0);

  LoggerAdapter::logSingle(DBG, "[EvLoop] Available methods are:");
  auto ** methods = event_get_supported_methods();
  for (int i = 0; methods[i] != nullptr; ++i)
  {
    LoggerAdapter::logSingle(DBG, "[EvLoop] Method: {}", methods[i]);
  }

  sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = hostPort;
  saddr.sin_addr.s_addr = hostIP;

  SocksSessionMng mng(saddr);
  int ret = mng.run();

  libevent_global_shutdown();
  
#ifdef _WIN32
  WSACleanup();
#endif
  return ret;
}
