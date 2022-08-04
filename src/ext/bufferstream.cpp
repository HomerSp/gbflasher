#include "ext/bufferstream.h"

using namespace ext;

BufferStream::BufferStream()
    : mOffset(0)
    , mBitOffset(0)
{
}

BufferStream::BufferStream(std::vector<uint8_t> data)
    : mOffset(0)
    , mBitOffset(0)
    , mData(std::move(data))
{
}

BufferStream::BufferStream(uint32_t count, uint8_t d)
    : BufferStream()
{
    mData.resize(count, d);
}

const std::vector<uint8_t> BufferStream::data() const
{
    return mData;
}

BufferStream& BufferStream::pad(uint64_t size, uint8_t c)
{
    if (mData.size() < size) {
        mData.resize(size, c);
    }

    return *this;
}

bool BufferStream::readBool()
{
    return bitRead(8) != 0x00U;
}

uint8_t BufferStream::readUInt8()
{
    return bitRead(8);
}

uint16_t BufferStream::readUInt16()
{
    return bitRead(8) | (bitRead(8) << 8U);
}

uint32_t BufferStream::readUInt24()
{
    return bitRead(8) | (bitRead(8) << 8U) | (bitRead(8) << 16U);
}

uint32_t BufferStream::readUInt32()
{
    return bitRead(8) | (bitRead(8) << 8U) | (bitRead(8) << 16U) | (bitRead(8) << 24U);
}

uint64_t BufferStream::readUInt64()
{
    return bitRead(8) | (bitRead(8) << 8U) | (bitRead(8) << 16U) | (bitRead(8) << 24U) | (bitRead(8) << 32U) | (bitRead(8) << 40U) | (bitRead(8) << 48U) | (bitRead(8) << 56U);
}

std::vector<uint8_t> BufferStream::readBytes(uint32_t count)
{
    std::vector<uint8_t> ret;
    if (count == -1) {
        while (!eof()) {
            ret.push_back(bitRead(8));
        }
    } else {
        for (uint32_t i = 0; i < count && !eof(); i++) {
            ret.push_back(bitRead(8));
        }
    }

    return ret;
}

std::string BufferStream::readString(uint32_t len, bool skipNull)
{
    std::string ret;
    if (len == -1) {
        while (!eof()) {
            uint8_t c = bitRead(8);
            if (c == 0x00) {
                break;
            }

            ret += static_cast<char>(c);
        }

    } else {
        for (uint32_t i = 0; i < len && !eof(); i++) {
            auto c = static_cast<char>(bitRead(8));
            if (skipNull && c == '\0') {
                continue;
            }

            ret += c;
        }
    }

    return ret;
}

BufferStream& BufferStream::append(uint8_t data)
{
    return bitAppend(data, 8);
}

BufferStream& BufferStream::append(uint16_t data)
{
    return bitAppend(data, 8 * 2);
}

BufferStream& BufferStream::append(uint32_t data)
{
    return bitAppend(data, 8 * 4);
}

BufferStream& BufferStream::append(uint64_t data)
{
    return bitAppend(data, 8 * 8);
}

BufferStream& BufferStream::append(const std::string& data, bool cstr)
{
    for (char c: data) {
        bitAppend(c, 8);
    }
    
    if (cstr) {
        bitAppend(0, 8);
    }

    return *this;
}

BufferStream& BufferStream::append(const std::vector<uint8_t>& data, uint32_t spos, uint32_t slen)
{
    if (slen == 0) {
        slen = data.size();
    }

    if (spos + slen > data.size()) {
        return *this;
    }

    for (uint32_t i = 0; i < slen; ++i) {
        bitAppend(data.at(spos + i), 8);
    }

    return *this;
}

BufferStream& BufferStream::append(const BufferStream& data)
{
    if (data.mBitOffset == 0) {
        return append(data.mData, 0, 0);
    }

    BufferStream stream = data;
    auto off = stream.bitOffset();

    stream.reset();
    while (off >= 8U) {
        bitAppend(stream.bitRead(8U), 8U);
        off -= 8U;
    }

    if (off > 0U) {
        bitAppend(stream.bitRead(off), off);
    }

    return *this;
}

BufferStream& BufferStream::appendWord(uint16_t data)
{
    bitAppend(data, 8);
    return bitAppend(data >> 8U, 8);
}

BufferStream& BufferStream::appendSword(uint32_t data)
{
    bitAppend(data, 8);
    bitAppend(data >> 8U, 8);
    return bitAppend(data >> 16U, 8);
}

BufferStream& BufferStream::appendDword(uint32_t data)
{
    bitAppend(data, 8);
    bitAppend(data >> 8U, 8);
    bitAppend(data >> 16U, 8);
    return bitAppend(data >> 24U, 8);
}

BufferStream& BufferStream::appendQword(uint64_t data)
{
    bitAppend(data, 8);
    bitAppend(data >> 8U, 8);
    bitAppend(data >> 16U, 8);
    bitAppend(data >> 24U, 8);
    bitAppend(data >> 32U, 8);
    bitAppend(data >> 40U, 8);
    bitAppend(data >> 48U, 8);
    return bitAppend(data >> 56U, 8);
}

uint64_t BufferStream::bitRead(uint64_t count)
{
    uint64_t ret = 0;
    if (count <= sizeof(uint64_t) * 8) {
        // Do a byte read
        if (mBitOffset == 0 && count % 8 == 0) {
            uint8_t byteCount = count / 8;
            for (int8_t i = 0; i < byteCount; i++) {
                ret <<= 8ULL;
                ret |= mData.at(mOffset + i) & 0xFFULL;
            }

            mOffset += byteCount;
        } else {
            uint64_t offset = 0;
            while (count > 0) {
                uint64_t bitOffset = (mOffset * 8) + mBitOffset + offset;
                uint32_t byteOffset = static_cast<uint64_t>(std::floor(bitOffset / 8.0f));
                if (byteOffset >= mData.size()) {
                    return 0;
                }

                uint64_t bitsLeft = 8 - bitOffset % 8;
                const uint8_t curData = mData.at(byteOffset) << (8 - bitsLeft);

                uint64_t bitsUsed = (bitsLeft < count) ? bitsLeft : count;
                if (bitsUsed <= 8) {
                    ret <<= bitsUsed;
                    uint64_t mask = (bitsUsed < sizeof(uint64_t)) ? ((1ULL << bitsUsed) - 1U) : 0xFFFFFFFFFFFFFFFF;
                    uint8_t off = 8U - bitsUsed;
                    ret |= (curData & (mask << off)) >> off;
                }

                count -= bitsUsed;
                offset += bitsUsed;
            }

            bitSkip(offset);
        }
    } else {
        bitSkip(count);
    }

    return ret;
}

uint64_t BufferStream::bitPeek(uint64_t count)
{
    uint64_t savedBit = mBitOffset;
    uint64_t savedOffset = mOffset;
    uint64_t ret = bitRead(count);
    mBitOffset = savedBit;
    mOffset = savedOffset;
    return ret;
}

BufferStream& BufferStream::bitAppend(uint64_t value, uint64_t count)
{
    uint64_t needSize = mOffset + std::ceil((mBitOffset + count) / 8.0f);
    if (mData.size() < needSize) {
        mData.resize(needSize);
    }

    mCurrentBitSize = (mOffset * 8 + mBitOffset + count) % 8;

    uint64_t offset = 0;
    while (count > 0) {
        uint64_t bitOffset = (mOffset * 8) + mBitOffset + offset;
        uint64_t bitsLeft = 8 - bitOffset % 8;

        uint64_t bitsUsed = bitsLeft < count ? bitsLeft : count;
        const uint64_t mask = (bitsUsed < sizeof(uint64_t)) ? ((1ULL << bitsUsed) - 1U) : 0xFFFFFFFFFFFFFFFF;
        const uint8_t value_bits = (value >> (count - bitsUsed)) & mask;

        uint8_t &curData = mData.at(bitOffset / 8);
        uint64_t off = bitsLeft - bitsUsed;
        curData &= ~(mask << off);
        curData |= value_bits << off;
        count -= bitsUsed;
        offset += bitsUsed;
    }

    bitSkip(offset);
    return *this;
}

BufferStream& BufferStream::bitFill(uint64_t count, uint8_t v)
{
    uint64_t value = 0;
    if (v > 0) {
        for (uint64_t i = 0; i < count; i++) {
            value |= 1 << i;
        }
    }

    return bitAppend(value, count);
}

BufferStream& BufferStream::bitSkip(int64_t count)
{
    if (count == 0) {
        if (mBitOffset > 0) {
            mOffset++;
        }

        mBitOffset = 0;
    } else {
        mOffset += std::floor((mBitOffset + count) / 8.0f);
        mBitOffset = (mBitOffset + count) % 8;
    }

    return *this;
}

BufferStream& BufferStream::bitReset(uint64_t pos)
{
    mOffset = std::floor(pos / 8);
    mBitOffset = pos % 8;
    return *this;
}
