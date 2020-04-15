#include "SignalConnectionStorage.h"

SignalConnectionStorage & SignalConnectionStorage::instance()
{
  static SignalConnectionStorage instance;
  return instance;
}

bool SignalConnectionStorage::hasConnection(uint32_t hash) const
{
  return _sigMap.find(hash) != _sigMap.end();
}

auto SignalConnectionStorage::getConnection(uint32_t hash) -> SignalConnection &
{
  auto it = _sigMap.find(hash);
  if (it == _sigMap.end())
    throw std::runtime_error{"Cannot find connection for pointer"};
  return it->second;
}
