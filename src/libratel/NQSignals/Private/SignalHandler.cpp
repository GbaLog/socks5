#include "SignalHandler.h"

SignalHandler::SignalHandler() :
  _runFlag(false),
  _error(Error::NoError)
{}

int SignalHandler::run(bool inNewThread)
{
  if (_runFlag)
    return 1;

  _runFlag = true;
  if (inNewThread)
  {
    _thread = std::thread(&SignalHandler::runImpl, this, false);
    return 0;
  }
  else
    return runImpl(false);
}

int SignalHandler::runOnce()
{
  return runImpl(true);
}

void SignalHandler::stop()
{
  _runFlag = false;
}

auto SignalHandler::getError() const -> Error
{
  return _error;
}

int SignalHandler::runImpl(bool once)
{
  if (once)
  {
    ISignalStorage * sig = nullptr;
    while (_queue.empty() == false)
    {
      sig = _queue.pop();
      executeSignal(sig);
    }
    return static_cast<int>(getError());
  }

  while (_runFlag)
  {
    if (!checkQueue())
      break;
  }
  return static_cast<int>(getError());
}

bool SignalHandler::checkQueue()
{
  ISignalStorage * sig = _queue.pop();
  if (!sig)
  {
    setError(Error::EmptySignal);
    return false;
  }
  executeSignal(sig);
  return true;
}

void SignalHandler::executeSignal(ISignalStorage * sig)
{
  if (!sig)
    return;
  sig->call();
  delete sig;
}

void SignalHandler::setError(Error err)
{
  _error = err;
}


