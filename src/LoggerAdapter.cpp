#include "LoggerAdapter.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/pattern_formatter.h"
#include <string>

LoggerAdapter::ConsoleSinkPtr LoggerAdapter::_consoleSink = nullptr;
LoggerAdapter::FileSinkPtr    LoggerAdapter::_fileSink = nullptr;

LoggerAdapter::LoggerAdapter(const std::string & name) :
  _logger(createLogger(name))
{}

LoggerAdapter::LoggerAdapter(const std::string & name, uint32_t id) :
  _logger(createLogger(fmt::format("{}] [{}", name, id)))
{}

LoggerAdapter::LoggerAdapter(const std::string & name, const std::string & id) :
  _logger(createLogger(fmt::format("{}] [{}", name, id)))
{}

void LoggerAdapter::globalInit(const std::string & name, int maxSize, int maxFiles, bool enableCon)
{
  std::string filename = name + ".log";
  _fileSink = createFileSink(filename, maxSize, maxFiles);
  if (enableCon)
  {
    _consoleSink = createConsoleSink();
  }

  initDefaultLogger();
}

void LoggerAdapter::globalInit(const std::string & name)
{
  _fileSink = nullptr;
  _consoleSink = createConsoleSink();

  initDefaultLogger();
}

void LoggerAdapter::globalSetLevel(int level)
{
  if (_fileSink)
    _fileSink->set_level(fromIntToLevel(level));
  if (_consoleSink)
    _consoleSink->set_level(fromIntToLevel(level));
}

void LoggerAdapter::setLevel(int level)
{
  _logger.set_level(fromIntToLevel(level));
}

spdlog::level::level_enum LoggerAdapter::fromIntToLevel(int level)
{
  switch (level)
  {
  case VRB:  return spdlog::level::trace;
  case DBG:  return spdlog::level::debug;
  case INF:  return spdlog::level::info;
  case WRN:  return spdlog::level::warn;
  case ERR:  return spdlog::level::err;
  case CRIT: return spdlog::level::critical;
  default: return spdlog::level::off;
  }
}

std::vector<spdlog::sink_ptr> LoggerAdapter::makeSinks()
{
  std::vector<spdlog::sink_ptr> sinks;
  if (_fileSink != nullptr)
    sinks.push_back(_fileSink);
  if (_consoleSink != nullptr)
    sinks.push_back(_consoleSink);
  return sinks;
}

void LoggerAdapter::initDefaultLogger()
{
  auto globalLogger = createLogger("global");
  spdlog::set_default_logger(globalLogger.clone("global"));
  logSingle(INF, "***** Start logging *****");
  logSingle(INF, "***** BUILD DATE: {}. BUILD TIME: {} *****", __DATE__, __TIME__);
}

spdlog::logger LoggerAdapter::createLogger(std::string name)
{
  auto sinks = makeSinks();
  spdlog::logger logger = spdlog::logger(std::move(name), sinks.begin(), sinks.end());
  logger.set_level(spdlog::level::trace);
  return logger;
}

LoggerAdapter::ConsoleSinkPtr LoggerAdapter::createConsoleSink()
{
  ConsoleSinkPtr sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  sink->set_level(spdlog::level::trace);
  sink->set_pattern("%T [%^%-5!l%$] [%n] %v");
  return sink;
}

LoggerAdapter::FileSinkPtr LoggerAdapter::createFileSink(const std::string & filename, int maxSize, int maxFiles)
{
  FileSinkPtr sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, maxSize, maxFiles, true);
  sink->set_level(spdlog::level::trace);
  sink->set_pattern("%T-%d:%m:%C [%^%-5!l%$] [%n] %v");
  return sink;
}
