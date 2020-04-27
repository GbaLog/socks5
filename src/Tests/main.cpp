#include <gtest/gtest.h>
#include "TracerConfig.h"
#include "ConsoleWriter.h"
#include "FileWriter.h"
#include "Tracer.h"

static
bool initLogging()
{
  FileWriterParams fileWrParams;
  fileWrParams._filePattern = "socks5_test.log";
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
  initLogging();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
