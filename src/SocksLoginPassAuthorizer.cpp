#include "SocksLoginPassAuthorizer.h"
#include "StringUtils.h"
#include <fstream>

SocksLoginPassAuthorizer::SocksLoginPassAuthorizer(const std::string & filename) :
  Traceable("LogPassAuth"),
  _filename(filename)
{
  if (readFile() == false)
    throw std::runtime_error{"Can't read file with logins and passwords: " + _filename};
}

bool SocksLoginPassAuthorizer::readFile()
{
  std::ifstream istrm{_filename};
  if (!istrm)
    return false;

  std::string line;
  while (std::getline(istrm, line))
  {
    auto strings = stringSplit(line, ",");
    if (strings.size() < 2)
      continue;
    _userLoginPass[strings[0]] = strings[1];
  }
  return true;
}

bool SocksLoginPassAuthorizer::isMethodSupported(const SocksAuthMethod & method) const
{
  return method._value == SocksAuthMethod::AuthLoginPass;
}

bool SocksLoginPassAuthorizer::authUserPassword(const std::string & user, const std::string & password) const
{
  TRACE(DBG) << "Check user auth for user: " << user << ", pass: " << password;
  auto it = _userLoginPass.find(user);
  if (it == _userLoginPass.end())
    return false;
  TRACE(DBG) << "User found, password: " << it->second;
  return it->second == password;
}
