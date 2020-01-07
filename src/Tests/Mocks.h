#ifndef MocksH
#define MocksH

#include <gmock/gmock.h>
#include "SocksInterfaces.h"

class SocksConnectionMock : public ISocksConnection
{
public:
  MOCK_METHOD0(connect, bool ());
  MOCK_METHOD1(send, bool (const VecByte & buf));
  MOCK_METHOD0(closeConnection, void ());
};

class SocksSessionUserMock : public ISocksSessionUser
{
public:
  MOCK_METHOD2(createNewConnection, ISocksConnectionPtr (SocksAddressType, const SocksAddress &));
};

class SocksAuthorizerMock : public ISocksAuthorizer
{
public:
  MOCK_METHOD2(authUserPassword, bool (const std::string & user, const std::string & pass));
};

#endif // MocksH
