#ifndef LoggerAdapterH
#define LoggerAdapterH
//-----------------------------------------------------------------------------
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "LoggerAdapterTypes.h"
//-----------------------------------------------------------------------------
enum
{
  VRB  = SPDLOG_LEVEL_TRACE,
  DBG  = SPDLOG_LEVEL_DEBUG,
  INF  = SPDLOG_LEVEL_INFO,
  WRN  = SPDLOG_LEVEL_WARN,
  ERR  = SPDLOG_LEVEL_ERROR,
  CRIT = SPDLOG_LEVEL_CRITICAL
};
//-----------------------------------------------------------------------------
class LoggerAdapter
{
public:
  LoggerAdapter(const std::string & name);
  LoggerAdapter(const std::string & name, uint32_t id);
  LoggerAdapter(const std::string & name, const std::string & id);

  static void globalInit(const std::string & name, int maxSize, int maxFiles, bool enableCon = true);
  static void globalInit(const std::string & name);
  static void globalSetLevel(int level);

  void setLevel(int level);

  template<class ... Args>
  void log(int level, const char * fmt, Args &&... args) const
  {
    _logger.log(fromIntToLevel(level), fmt, std::forward<Args>(args)...);
  }

  template<class ... Args>
  static void logSingle(int level, const char * fmt, Args &&... args)
  {
    spdlog::log(fromIntToLevel(level), fmt, std::forward<Args>(args)...);
  }

private:
  typedef std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> FileSinkPtr;
  typedef std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> ConsoleSinkPtr;
  static ConsoleSinkPtr _consoleSink;
  static FileSinkPtr    _fileSink;

  mutable spdlog::logger _logger;

  static spdlog::level::level_enum fromIntToLevel(int level);
  static std::vector<spdlog::sink_ptr> makeSinks();
  static void initDefaultLogger();
  static spdlog::logger createLogger(std::string name);
  static ConsoleSinkPtr createConsoleSink();
  static FileSinkPtr    createFileSink(const std::string & filename, int maxSize, int maxFiles);
};
//-----------------------------------------------------------------------------
#endif // LoggerAdapterH
//-----------------------------------------------------------------------------
