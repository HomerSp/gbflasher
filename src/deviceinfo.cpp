#include "deviceinfo.h"
#include "flashfile.h"
#include "ext/bufferstream.h"
#include "utils/hex.h"

DeviceInfo::DeviceInfo(const std::vector<uint8_t>& data)
{
    ext::BufferStream stream(data);
    mFieldSize = stream.readByte();
    mBytesPerAddress = stream.readByte();
    for (uint32_t i = 0; i < 6; ++i) {
        MemoryInfo::Type type = static_cast<MemoryInfo::Type>(stream.readByte());
        if (type == MemoryInfo::END) {
            break;
        }

        uint32_t addr = stream.readDword();
        uint32_t len = stream.readDword();
        //printf("Device memory type %02X, %08X, %08X\n", type, addr, len);
        mMemInfo.emplace_back(type, addr, len);
    }
}

uint32_t DeviceInfo::address(MemoryInfo::Type type) const
{
    for (const auto& p: mMemInfo) {
        if (p.type == type) {
            return p.start;
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
        if (addr >= p.start && addr + size <= p.start + p.length && (maxLen == 0 || p.length <= maxLen)) {
            //printf("DeviceInfo::memoryType %08X vs %08X - %08X, type %d\n", addr, p.start, p.start + p.length, p.type);
            ret = p.type;
            maxLen = p.length;
        }
    }

    return ret;
}
