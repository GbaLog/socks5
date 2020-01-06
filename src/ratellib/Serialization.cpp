#include "Serialization.h"
//-----------------------------------------------------------------------------
SerializeBuffer::SerializeBuffer(const Byte * buffer, size_t size) :
  _reader(buffer, size),
  _writer(nullptr, 0)
{}
//-----------------------------------------------------------------------------
SerializeBuffer::SerializeBuffer(Byte * buffer, size_t size) :
  _reader(buffer, size),
  _writer(buffer, size)
{}
//-----------------------------------------------------------------------------
namespace Serialize
{
//-----------------------------------------------------------------------------
bool read(MemReader & reader, bool & val)
{
  uint8_t tmp;
  bool res = reader.readUint8(tmp);
  val = tmp;
  return res;
}
//-----------------------------------------------------------------------------
bool read(MemReader & reader, uint8_t & val)
{
  return reader.readUint8(val);
}
//-----------------------------------------------------------------------------
bool read(MemReader & reader, uint16_t & val)
{
  return reader.readUint16(val);
}
//-----------------------------------------------------------------------------
bool read(MemReader & reader, uint32_t & val)
{
  return reader.readUint32(val);
}
//-----------------------------------------------------------------------------
bool read(MemReader & reader, std::string & str)
{
  uint16_t size;
  if (read(reader, size) == false)
    return false;

  str.resize(size);
  return reader.readData((uint8_t *)str.data(), size);
}
//-----------------------------------------------------------------------------
} //namespace Serialize
//-----------------------------------------------------------------------------
