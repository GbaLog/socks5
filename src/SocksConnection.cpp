#include "SocksConnection.h"

SocksConnection::SocksConnection(ISocksConnectionUser & user,
                                 const SocksAddress & localAddr, const SocksAddress & remoteAddr) :
  _user(user)
{}
