#include "Tracer.h"
#include <ctime>
#include <iomanip>
//-----------------------------------------------------------------------------
TempTracer::TempTracer(const std::string & lvl, bool addEnd, std::ostream & strm) :
  _strm(strm),
  _addEnd(addEnd)
{
  char buf[32] = {};
  time_t t = time(nullptr);
  tm * ptime = nullptr;
  ptime = localtime(&t);
  strftime(buf, sizeof(buf), "%y%m%d-%H%M%S", ptime);
  _strm << buf << " [" << lvl << "] ";
}
//-----------------------------------------------------------------------------
TempTracer::~TempTracer()
{
  if (_addEnd)
    _strm << "\n";
}
//-----------------------------------------------------------------------------
TempTracer::TempTracer(TempTracer && rhs) :
  _strm(rhs._strm),
  _addEnd(true)
{}
//-----------------------------------------------------------------------------
TraceObject::TraceObject(const std::string & name, const std::string & id) :
  _name(name),
  _id(id)
{}
//-----------------------------------------------------------------------------
TraceObject::TraceObject(const std::string & name, int id) :
  _name(name),
  _id(std::to_string(id))
{}
//-----------------------------------------------------------------------------
TempTracer TraceObject::makeTrace(const std::string & lvl)
{
  TempTracer trace(lvl, false, std::cout);
  trace << std::left;
  trace << "| " << std::setw(12) << _name << " | "
        << std::setw(12) << _id << " | ";
  trace << std::right;
  return std::move(trace);
}
//-----------------------------------------------------------------------------
Traceable::Traceable(const std::string & name, const std::string & id) :
  _obj(name, id)
{}
//-----------------------------------------------------------------------------
Traceable::Traceable(const std::string & name, int id) :
  _obj(name, id)
{}
//-----------------------------------------------------------------------------
TempTracer Traceable::makeTrace(const std::string & lvl)
{
  return _obj.makeTrace(lvl);
}
//-----------------------------------------------------------------------------
std::string getLvlStrByInt(int type)
{
  switch (type)
  {
  case DBG: return "DBG";
  case WRN: return "WRN";
  case INF: return "INF";
  case ERR: return "ERR";
  default:  return "UNK";
  }
}
