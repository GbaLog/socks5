#include "SocksSession.h"

SocksSession::SocksSession(ISocksSessionUser & user, ISocksConnection & incoming, ISocksAuthorizer & auth) :
  _user(user),
  _inConnection(incoming),
  _auth(auth),
  _decoder({ SocksVersion::Version5 })
{}

void SocksSession::processData(const VecByte & buf)
{
}

void SocksSession::clientDisconnected()
{

}

void SocksSession::onReceive(const VecByte & buf)
{

}

void SocksSession::onConnectionClosed()
{

}
