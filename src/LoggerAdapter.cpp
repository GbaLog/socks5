#include "LoggerAdapter.h"
#include "spdlog/fmt/fmt.h"

LoggerAdapter::ConsoleSinkPtr LoggerAdapter::_consoleSink = nullptr;
LoggerAdapter::FileSinkPtr    LoggerAdapter::_fileSink = nullptr;

LoggerAdapter::LoggerAdapter(const std::string & name) :
  _logger(name, {_consoleSink, _fileSink})
{}

LoggerAdapter::LoggerAdapter(const std::string & name, uint32_t id) :
  _logger(fmt::format("{}/{}", name, id), {_consoleSink, _fileSink})
{}

LoggerAdapter::LoggerAdapter(const std::string & name, const std::string & id) :
  _logger(fmt::format("{}/{}", name, id), {_consoleSink, _fileSink})
{}

void LoggerAdapter::globalInit(const std::string & name, int maxSize, int maxFiles)
{
  std::string filename = name + ".log";
  _fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, maxSize, maxFiles, true);
  _fileSink->set_level(spdlog::level::trace);
  _consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  _consoleSink->set_level(spdlog::level::trace);
  logSingle(INF, "***** Start logging *****");
}

void LoggerAdapter::globalInit(const std::string & name)
{
  _fileSink = nullptr;
  _consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  _consoleSink->set_level(spdlog::level::trace);
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
