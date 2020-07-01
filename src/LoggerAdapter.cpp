#include "LoggerAdapter.h"
#include "spdlog/fmt/fmt.h"

LoggerAdapter::ConsoleSinkPtr LoggerAdapter::_consoleSink = nullptr;
LoggerAdapter::FileSinkPtr    LoggerAdapter::_fileSink = nullptr;

LoggerAdapter::LoggerAdapter(const std::string & name) :
  _logger(createLogger(name))
{
  _logger.set_level(spdlog::level::trace);
}

LoggerAdapter::LoggerAdapter(const std::string & name, uint32_t id) :
  _logger(createLogger(fmt::format("{}/{}", name, id)))
{
  _logger.set_level(spdlog::level::trace);
}

LoggerAdapter::LoggerAdapter(const std::string & name, const std::string & id) :
  _logger(createLogger(fmt::format("{}/{}", name, id)))
{
  _logger.set_level(spdlog::level::trace);
}

void LoggerAdapter::globalInit(const std::string & name, int maxSize, int maxFiles, bool enableCon)
{
  std::string filename = name + ".log";
  _fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, maxSize, maxFiles, true);
  _fileSink->set_level(spdlog::level::trace);
  if (enableCon)
  {
    _consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    _consoleSink->set_level(spdlog::level::trace);
    auto globalLogger = spdlog::logger("global", {_consoleSink, _fileSink});
    globalLogger.set_level(spdlog::level::trace);
    spdlog::set_default_logger(globalLogger.clone("global"));
  }
  else
  {
    auto globalLogger = spdlog::logger("global", {_fileSink});
    globalLogger.set_level(spdlog::level::trace);
    spdlog::set_default_logger(globalLogger.clone("global"));
  }
  logSingle(INF, "***** Start logging *****");
}

void LoggerAdapter::globalInit(const std::string & name)
{
  _fileSink = nullptr;
  _consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  _consoleSink->set_level(spdlog::level::trace);
  auto globalLogger = spdlog::logger("global", {_consoleSink});
  globalLogger.set_level(spdlog::level::trace);
  spdlog::set_default_logger(globalLogger.clone("global"));
  logSingle(INF, "***** Start logging *****");
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

spdlog::logger LoggerAdapter::createLogger(std::string name)
{
  auto sinks = makeSinks();
  return spdlog::logger(std::move(name), sinks.begin(), sinks.end());
}
