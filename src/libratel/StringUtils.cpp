#include "StringUtils.h"


VecString stringSplit(const std::string & str, const std::string & delim)
{
  VecString res;

  auto it = str.find(delim);
  res.push_back(str.substr(0, it));

  auto prev = it + delim.size();
  while (it != std::string::npos)
  {
    it = str.find(delim, prev);
    res.push_back(str.substr(prev, (it - prev)));
    prev += delim.size() + res.back().size();
  }

  return res;
}
