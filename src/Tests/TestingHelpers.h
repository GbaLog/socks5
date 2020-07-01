#ifndef TestingHelpersH
#define TestingHelpersH

#include "gtest/gtest.h"
#include "LoggerAdapter.h"

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
    LoggerAdapter::globalInit(file, maxSize, maxFiles, false);
  }
};

#endif // TestingHelpersH
