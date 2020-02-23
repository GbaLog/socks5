#ifndef RatelTracerH
#define RatelTracerH
//-----------------------------------------------------------------------------
#include <string>
#include <iostream>
//-----------------------------------------------------------------------------
enum LogLevel
{
  DBG,
  WRN,
  INF,
  ERR
};
//-----------------------------------------------------------------------------
class TempTracer
{
public:
  TempTracer(const std::string & lvl, bool addEnd, std::ostream & strm);
  ~TempTracer();
  TempTracer(TempTracer && rhs);

  template<class T>
  TempTracer & operator <<(const T & val)
  {
    _strm << val;
    return *this;
  }

private:
  std::ostream & _strm;
  bool _addEnd;
};
//-----------------------------------------------------------------------------
class TraceObject
{
public:
  TraceObject(const std::string & name, const std::string & id);
  TraceObject(const std::string & name, int id);

  TempTracer makeTrace(const std::string & lvl);
  void setId(const std::string & id);
  void setId(int id);

private:
  std::string _name;
  std::string _id;
};
//-----------------------------------------------------------------------------
class Traceable
{
public:
  Traceable(const std::string & name, const std::string & id = "");
  Traceable(const std::string & name, int id);

  TempTracer makeTrace(const std::string & lvl);
  void setId(const std::string & id);
  void setId(int id);

protected:
  mutable TraceObject _traceObj;
};
//-----------------------------------------------------------------------------
std::string getLvlStrByInt(int type);
//-----------------------------------------------------------------------------
#define TRACE(lvl) (this->_traceObj).makeTrace(getLvlStrByInt(lvl))
#define TRACE_SINGLE(lvl, name) TraceObject(name, "").makeTrace(getLvlStrByInt(lvl))
//-----------------------------------------------------------------------------
#endif // RatelTracerH
//-----------------------------------------------------------------------------
