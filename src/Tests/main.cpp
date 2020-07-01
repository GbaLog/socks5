#include <gtest/gtest.h>
#include "TestingHelpers.h"

int main(int argc, char * argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  auto * logListener = new LogListener;
  logListener->setUpLogger("socks5_test", 1024 * 1024 * 5, 10);

  auto & listeners = ::testing::UnitTest::GetInstance()->listeners();
  listeners.Append(logListener);

  return RUN_ALL_TESTS();
}
