#include "deviceinfo.h"
#include "ext/bufferstream.h"
#include "flashfile.h"
#include "logger.h"
#include "utils/hex.h"

DeviceInfo::DeviceInfo(const std::vector<uint8_t>& data)
{
    ext::BufferStream stream(data);
    mFieldSize = stream.readUInt8();
    mBytesPerAddress = stream.readUInt8();
    for (uint32_t i = 0; i < 6; ++i) {
        MemoryInfo::Type type = static_cast<MemoryInfo::Type>(stream.readUInt8());
        if (type == MemoryInfo::END) {
            break;
        }

        uint32_t addr = stream.readUInt32();
        uint32_t len = stream.readUInt32();
        Logger::verbose<DeviceInfo>("DeviceInfo")("Memory type %02X, address %08X, length %08X", type, addr, len);
        mMemInfo.emplace_back(type, addr, len);
    }
}

uint32_t DeviceInfo::address(MemoryInfo::Type type) const
{
    for (const auto& p: mMemInfo) {
        if (p.type == type) {
            return p.address;
        }
    }

    return 0xFFFFFFFFU;
}

MemoryInfo::Type DeviceInfo::memoryType(uint32_t addr, uint32_t size) const
{
    MemoryInfo::Type ret = MemoryInfo::NONE;
    uint32_t maxLen = 0;
    for (const auto& p: mMemInfo) {
        // Find the shortest matching block
        if (addr >= p.address && addr + size <= p.address + p.length && (maxLen == 0 || p.length <= maxLen)) {
            ret = p.type;
            maxLen = p.length;
        }
    }

    return ret;
}
