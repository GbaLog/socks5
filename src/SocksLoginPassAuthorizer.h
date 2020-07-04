#ifndef SocksLoginPassAuthorizerH
#define SocksLoginPassAuthorizerH
//-----------------------------------------------------------------------------
#include <unordered_map>
#include "SocksInterfaces.h"
#include "SocksTypes.h"
#include "LoggerAdapter.h"
//-----------------------------------------------------------------------------
class SocksLoginPassAuthorizer : public ISocksAuthorizer, private LoggerAdapter
{
public:
  explicit SocksLoginPassAuthorizer(const std::string & filename);

private:
  std::string _filename;
  typedef std::unordered_map<std::string, std::string> MapLoginPassword;
  MapLoginPassword _userLoginPass;

  bool readFile();

  //ISocksAuthorizer
  virtual bool isMethodSupported(const SocksAuthMethod & method) const override;
  virtual bool authUserPassword(const std::string & user, const std::string & password) const override;
};
//-----------------------------------------------------------------------------
#endif // SocksLoginPassAuthorizerH
//-----------------------------------------------------------------------------
