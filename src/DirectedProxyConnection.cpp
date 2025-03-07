#include "DirectedProxyConnection.h"
//-----------------------------------------------------------------------------
DirectedProxyConnection::DirectedProxyConnection(uint32_t id, IDirectedProxyConnectionOwner & owner, ProxyDirection direction, SocksConnectionPtr conn) :
  LoggerAdapter("ProxyConn", formatLoggerId(id, direction)),
  _owner(owner),
  _direction(direction),
  _connection(conn)
{
  _connection->setUser(this);
}
//-----------------------------------------------------------------------------
DirectedProxyConnection::~DirectedProxyConnection()
{
  if (_connection->isConnected())
    _connection->closeConnection();
  _connection->setUser(nullptr);
}
//-----------------------------------------------------------------------------
bool DirectedProxyConnection::send(const VecByte & buf)
{
  return _connection->send(buf);
}
//-----------------------------------------------------------------------------
void DirectedProxyConnection::disconnect()
{
  _connection->closeConnection();
}
//-----------------------------------------------------------------------------
void DirectedProxyConnection::onReceive(const VecByte & buf)
{
  _owner.onDataReceived(_direction, buf);
}
//-----------------------------------------------------------------------------
void DirectedProxyConnection::onConnected()
{
  _owner.onConnected(_direction);
}
//-----------------------------------------------------------------------------
void DirectedProxyConnection::onConnectionClosed()
{
  _owner.onDisconnected(_direction);
}
//-----------------------------------------------------------------------------
std::string DirectedProxyConnection::formatLoggerId(uint32_t id, ProxyDirection direction)
{
  switch (direction)
  {
  case ProxyDirection::Main:
    return fmt::format("{}/main", id);
  case ProxyDirection::In:
    return fmt::format("{}/in", id);
  case ProxyDirection::Out:
    return fmt::format("{}/out", id);
  default:
    return fmt::format("{}/unk", id);
  }
}
//-----------------------------------------------------------------------------
