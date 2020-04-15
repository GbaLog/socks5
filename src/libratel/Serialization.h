#ifndef RatelSerializationH
#define RatelSerializationH
//-----------------------------------------------------------------------------
#include "Common.h"
#include "Optional.h"
#include "MemReader.h"
#include "MemWriter.h"
//-----------------------------------------------------------------------------
namespace Serialize
{
  bool read(MemReader & reader, bool & val);
  bool read(MemReader & reader, uint8_t & val);
  bool read(MemReader & reader, uint16_t & val);
  bool read(MemReader & reader, uint32_t & val);
  bool read(MemReader & reader, std::string & str);
  template<typename T>
  bool read(MemReader & reader, Optional<T> & val)
  {
    bool res;
    if (read(reader, res) == false)
      return false;

    T t;
    if (res)
    {
      res = read(reader, t);
      val.emplace(std::move(t));
    }
    return res;
  }

  template<class T>
  bool read(MemReader & reader, std::vector<T> & val)
  {
    uint16_t size;
    if (read(reader, size) == false)
      return false;

    val.resize(size);
    for (auto & it : val)
      if (read(reader, it) == false)
        return false;
    return true;
  }
}
//-----------------------------------------------------------------------------
class SerializeBuffer
{
public:
  SerializeBuffer(const Byte * buffer, size_t size);
  SerializeBuffer(Byte * buffer, size_t size);

  template<class T>
  bool read(T & val)
  {
    return Serialize::read(_reader, val);
  }

private:
  MemReader _reader;
  MemWriter _writer;
};
//-----------------------------------------------------------------------------
#endif // RatelSerializationH
//-----------------------------------------------------------------------------
