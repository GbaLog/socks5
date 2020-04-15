#include <cassert>
#include <cstring>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include "SocksSessionMng.h"
#include "Tracer.h"
#include "TracerConfig.h"
#include "ConsoleWriter.h"
#include "FileWriter.h"
#include "InetUtils.h"

extern "C"
void eventLog(int severity, const char * msg)
{
  TraceLevel lvl = TraceLevel::ERR;
  switch (severity)
  {
  case EVENT_LOG_DEBUG: lvl = TraceLevel::DBG; break;
  case EVENT_LOG_MSG:   lvl = TraceLevel::INF; break;
  case EVENT_LOG_WARN:  lvl = TraceLevel::WRN; break;
  case EVENT_LOG_ERR:   lvl = TraceLevel::ERR; break;
  default:              lvl = TraceLevel::ERR; break;
  }

  TRACE_SINGLE(lvl, "EvLog") << msg;
}

static
bool initLogging()
{
  FileWriterParams fileWrParams;
  fileWrParams._filePattern = "socks5.log";
  fileWrParams._maxBytes = 1024 * 1024 * 10; //10 MByte
  fileWrParams._maxFiles = 10;

  try
  {
    auto con = std::make_shared<ConsoleWriter>();
    TracerConfig::instance().addWriter(con);

    auto file = std::make_shared<FileWriter>(fileWrParams);
    TracerConfig::instance().addWriter(file);
  }
  catch (const std::runtime_error & ex)
  {
    TRACE_FORCE(ERR, "Init") << ex.what();
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
    TRACE_SINGLE(INF, "main") << "Usage: " << argv[0] << " [host] [port]";
    return 0;
  }

  uint32_t hostIP = 0;
  uint16_t hostPort = ratel::htons(35555);
  
  if (argc >= 2)
  {
    auto tmpAddr = inet_addr(argv[1]);
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

  TRACE_SINGLE(DBG, "EvLoop") << "Available methods are:";
  auto ** methods = event_get_supported_methods();
  for (int i = 0; methods[i] != nullptr; ++i)
  {
    TRACE_SINGLE(DBG, "EvLoop") << "Method: " << methods[i];
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
