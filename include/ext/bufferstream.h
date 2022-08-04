#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

constexpr uint8_t _byte(unsigned long long d)
{
    return static_cast<uint8_t>(d);
}

namespace ext {
class BufferStream {
public:
    BufferStream();
    BufferStream(std::vector<uint8_t> data);
    BufferStream(uint32_t count, uint8_t d);

    const std::vector<uint8_t> data() const;

    bool eof() const { return bitOffset() >= bitSize(); }
    uint64_t offset() const { return mOffset; }
    uint64_t size() const { return mData.size(); }

    void reserve(uint32_t count) { return mData.reserve(count); }

    BufferStream& skip(uint64_t count = 0) { return bitSkip(count * 8); }
    BufferStream& reset(uint64_t pos = 0) { return bitReset(pos * 8); }
    BufferStream& pad(uint64_t size, uint8_t c = 0xFFU);
    BufferStream& fill(uint64_t count, uint8_t c = 0x00U) { return bitFill(count * 8, c); }

    bool readBool();
    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt24();
    uint32_t readUInt32();
    uint64_t readUInt64();
    std::vector<uint8_t> readBytes(uint32_t count = -1);
    std::string readString(uint32_t len = -1, bool skipNull = true);

    BufferStream& append(uint8_t data);
    BufferStream& append(uint16_t data);
    BufferStream& append(uint32_t data);
    BufferStream& append(uint64_t data);
    BufferStream& append(const std::string& data, bool cstr = true);
    BufferStream& append(const std::vector<uint8_t>& data, uint32_t spos = 0, uint32_t slen = 0);
    BufferStream& append(const BufferStream& data);

    BufferStream& appendWord(uint16_t data);
    BufferStream& appendSword(uint32_t data);
    BufferStream& appendDword(uint32_t data);
    BufferStream& appendQword(uint64_t data);

    BufferStream& operator<<(uint8_t data) { return append(data); }
    BufferStream& operator<<(uint16_t data) { return append(data); }
    BufferStream& operator<<(uint32_t data) { return append(data); }
    BufferStream& operator<<(const std::string& data) { return append(data); }
    BufferStream& operator<<(const std::vector<uint8_t>& data) { return append(data); }
    BufferStream& operator<<(const BufferStream& data) { return append(data); }

    uint64_t bitOffset() const { return mOffset * 8 + mBitOffset; }
    uint64_t bitSize() const { return mData.size() * 8 - 8 + (mCurrentBitSize == 0 ? 8 : mCurrentBitSize); }
    uint64_t bitRemain() const { return bitSize() - bitOffset(); }

    BufferStream& bitSkip(int64_t count = 0);
    BufferStream& bitReset(uint64_t pos = 0);

    template<typename T>
    T bitRead(uint64_t count) { return static_cast<T>(bitRead(count)); }

    uint64_t bitRead(uint64_t count);
    uint64_t bitPeek(uint64_t count);

    BufferStream& bitAppend(uint64_t value, uint64_t count);
    BufferStream& bitFill(uint64_t count, uint8_t v);

private:
    uint64_t mOffset;
    uint8_t mBitOffset;
    std::vector<uint8_t> mData;
    uint8_t mCurrentBitSize = 0;
};
}
