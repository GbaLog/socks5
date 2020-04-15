#ifndef RatelMemReaderH
#define RatelMemReaderH
//-----------------------------------------------------------------------------
#include <cstdint>
#include <cstddef>
//-----------------------------------------------------------------------------
class MemReader
{
public:
  MemReader();
  MemReader(const uint8_t * data, size_t size);
  MemReader(const MemReader & rhs) = default;
  MemReader & operator =(const MemReader & rhs) = default;

  bool setPos(uint32_t pos);

  bool readUint8(uint8_t & val);
  bool readUint16(uint16_t & val);
  bool readUint32(uint32_t & val);
  bool readUint64(uint64_t & val);
  bool readData(uint8_t * data, size_t size);

  bool shift(int64_t val);
  bool shiftFromStart(uint32_t val);

  size_t size() const;
  uint32_t getOffset() const;
  uint32_t getRemainingSize() const;
  const uint8_t * getPos() const;

private:
  const uint8_t * _data;
  const size_t _size;
  const uint8_t * _pos;
};
//-----------------------------------------------------------------------------
#endif // RatelMemReaderH
//-----------------------------------------------------------------------------
