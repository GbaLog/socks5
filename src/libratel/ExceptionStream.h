#ifndef ExceptionStreamH
#define ExceptionStreamH
//-----------------------------------------------------------------------------
#include <sstream>
#include <string>
//-----------------------------------------------------------------------------
#define RATEL_THROW(ex_type) throw ExceptionStream<ex_type>(__FILE__, __LINE__, __func__)
#define RATEL_THROW_IF(c, ex) if (!(c)) ; else RATEL_THROW(ex)
//-----------------------------------------------------------------------------
template<class T>
class ExceptionStream : public T
{
public:
  ExceptionStream(const char * file, const int line, const char * func) : T("")
  {
    _strm << "File: " << file << ", line: " << line << ", func: " << func << " '";
  }

  ExceptionStream(const ExceptionStream & rhs) :
    T(rhs._strm.str() + '\''),
    _strm(rhs._strm.str()),
    _str(rhs._strm.str())
  {}

  template<class U>
  ExceptionStream & operator <<(const U & val)
  {
    _strm << val;
    return *this;
  }

  virtual const char * what() const noexcept override
  {
    _str = _strm.str();
    _str.push_back('\'');
    return _str.c_str();
  }

private:
  mutable std::ostringstream _strm;
  mutable std::string _str;
};
//-----------------------------------------------------------------------------
#endif //ExceptionStreamH
//-----------------------------------------------------------------------------
