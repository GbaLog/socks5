#ifndef TestingHelpersH
#define TestingHelpersH

#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

class LogListener : public ::testing::EmptyTestEventListener
{
public:
  virtual void OnTestStart(const ::testing::TestInfo & testInfo)
  {
    spdlog::info("*** Test {}.{} starting ***",
                 testInfo.test_suite_name(), testInfo.name());
  }

  virtual void OnTestEnd(const ::testing::TestInfo & testInfo)
  {
    spdlog::info("*** Test {}.{} ended ***",
                 testInfo.test_suite_name(), testInfo.name());
  }

  void setUpLogger(const std::string & file, int maxSize, int maxFiles)
  {
    auto logger = spdlog::rotating_logger_mt("Testing", file, maxSize, maxFiles, true);
    logger->info("***** Start logging *****");
    spdlog::set_default_logger(logger);
  }
};

#endif // TestingHelpersH
