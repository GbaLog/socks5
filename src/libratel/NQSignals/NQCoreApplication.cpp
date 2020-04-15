#include "NQCoreApplication.h"
#include "Private/SignalHandler.h"

NQCoreApplication::NQCoreApplication()
{

}

SignalHandler & NQCoreApplication::getSignalHandler()
{
  static SignalHandler handler;
  return handler;
}

int NQCoreApplication::runOnce()
{
  return getSignalHandler().runOnce();
}

int NQCoreApplication::run(bool inNewThread)
{
  return getSignalHandler().run(inNewThread);
}

void NQCoreApplication::stop()
{
  return getSignalHandler().stop();
}


