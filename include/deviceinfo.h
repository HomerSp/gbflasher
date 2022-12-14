#pragma once
#include <vector>

#include "memoryinfo.h"

class FlashFile;
class DeviceInfo {
public:
    static constexpr auto Tag = "DeviceInfo";

public:
    DeviceInfo(const std::vector<uint8_t>& data);

    const std::vector<MemoryInfo>& memInfo() const { return mMemInfo; }

    uint32_t address(MemoryInfo::Type type) const;
    MemoryInfo::Type memoryType(uint32_t addr, uint32_t size) const;

    bool validateFlashFile(const FlashFile& file) const;

private:
    uint8_t mFieldSize;
    uint8_t mBytesPerAddress;
    std::vector<MemoryInfo> mMemInfo;
};
